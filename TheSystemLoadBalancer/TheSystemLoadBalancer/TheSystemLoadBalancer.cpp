
#include <iostream>
#include <vector>
#include <thread>
#include <utility> // std::pair
#include <signal.h>
#include <fstream>
#include <string>
#include <algorithm>

#include "sockets.h"

volatile bool isRunning = true;
std::vector<std::pair<SOCKET, struct sockaddr_storage>> serverSockets;

const long SELECT_TIMEOUT_SEC = 1;
std::string LOAD_BALANCER_IP = "127.0.0.1";
std::string SERVER_IP = "127.0.0.1";
const int RESPONSE_TX_PORT = 3576; // SEND to client
const int RESPONSE_RX_PORT = 3577; // RECV from server
const int REQUEST_TX_PORT = 3578;  // SEND to server
const int REQUEST_RX_PORT = 3579;  // RECV from client

void signalHandler(int signal) {
	std::cout << "Handling signal. Wait up to " << SELECT_TIMEOUT_SEC << " seconds to finish." << std::endl;
	isRunning = false;
}

void cleanup() {
	isRunning = false;
#if defined(_WIN32)
	WSACleanup();
#endif
}

bool loadAddresses() {
	std::ifstream file;
	file.open("../../addresses.csv");
	if (!file.is_open()) {
		return false;
	}

	std::string line = "";
	const std::string delimiter = ",";
	while (std::getline(file, line)) {
		if (line.substr(0, line.find(delimiter)) == "load_balancer") {
			line.erase(0, line.find(delimiter) + delimiter.length()); // erase the first entry in the delimited string
			LOAD_BALANCER_IP = line.substr(0, line.find(delimiter));

			// trim whitespace from the right (especially CRLF)
			LOAD_BALANCER_IP.erase(std::find_if(LOAD_BALANCER_IP.rbegin(), LOAD_BALANCER_IP.rend(), [](unsigned char ch) {
				return !std::isspace(ch);
				}).base(), LOAD_BALANCER_IP.end());
			std::cout << "Setting own IP address to " << LOAD_BALANCER_IP << std::endl;
			break;
		}
	}

	file.close();
	return true;
}

bool createServerSocket(SOCKET &serverSocket) {
	// listen for servers trying to connect over TCP
	serverSocket = createSocket(AF_INET, SOCK_STREAM, 0);
	if (!isValidSocket(serverSocket)) {
		std::cout << "Failed to create the server socket" << std::endl;
		return false;
	}

	struct sockaddr_in addr;
	switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), RESPONSE_RX_PORT)) {
	case -1:
		std::cout << "Failed to make sockaddr_in for the server socket" << std::endl;
		return false;
	case 0:
		std::cout << "Invalid IP address for the server socket" << std::endl;
		return false;
	case 1:
	default:
		break;
	}

	if (SOCKET_ERROR == bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr))) {
		std::cout << "Failed to bind the server socket" << std::endl;
		return false;
	}

	if (SOCKET_ERROR == listen(serverSocket, 10)) {
		std::cout << "Failed to listen on the server socket" << std::endl;
		return false;
	}

	return true;
}

bool createClientSocket(SOCKET &clientSocket) {
	// communicate with clients over UDP
	clientSocket = createSocket(AF_INET, SOCK_DGRAM, 0);
	if (!isValidSocket(clientSocket)) {
		std::cout << "Failed to create the client socket" << std::endl;
		return false;
	}

	struct sockaddr_in addr;
	switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), REQUEST_RX_PORT)) {
	case -1:
		std::cout << "Failed to make sockaddr_in for the client socket" << std::endl;
		return false;
	case 0:
		std::cout << "Invalid IP address" << std::endl;
		return false;
	case 1:
	default:
		break;
	}

	if (SOCKET_ERROR == bind(clientSocket, (struct sockaddr *)&addr, sizeof(addr))) {
		std::cout << "Failed to bind the client socket" << std::endl;
		return false;
	}

	return true;
}

int main() {

	if (SIG_ERR == signal(SIGINT, signalHandler)) {
		std::cout << "Failed to set signal handler" << std::endl;
		return 1;
	}

	if (!loadAddresses()) {
		std::cout << "Failed to set IP addresses" << std::endl;
		return 1;
	}


	SOCKET serverSocket = -1;
	if (!createServerSocket(serverSocket)) {
		closeSocket(serverSocket);
		cleanup();
		return 1;
	}


	SOCKET clientSocket = -1;
	if (!createClientSocket(clientSocket)) {
		closeSocket(serverSocket);
		closeSocket(clientSocket);
		cleanup();
		return 1;
	}

	SOCKET maxFD = clientSocket;

	fd_set mainSet;
	FD_ZERO(&mainSet);
	FD_SET(serverSocket, &mainSet);
	FD_SET(clientSocket, &mainSet);

	while (isRunning) {
		fd_set readSet = mainSet;

		struct timeval tv;
		tv.tv_sec = SELECT_TIMEOUT_SEC;
		tv.tv_usec = 0;
		int ret = select(maxFD + 1, &readSet, nullptr, nullptr, &tv);
		if (-1 == ret) {
			// error
			std::cout << "Select error" << std::endl;
			continue;
		} else if (0 == ret) {
			// timeout
			continue;
		} else if (FD_ISSET(serverSocket, &readSet)) {
			// accept connections
			struct sockaddr_storage theirAddr;
			socklen_t theirSize = sizeof(theirAddr);
			SOCKET newSocket = accept(serverSocket, (struct sockaddr *)&theirAddr, &theirSize);
			if (INVALID_SOCKET == newSocket) {
				std::cout << "Server accept failed" << std::endl;
				continue;
			} else {
				std::cout << "New server accepted" << std::endl;
			}

			serverSockets.push_back(std::make_pair(newSocket, theirAddr));
			FD_SET(newSocket, &mainSet);
			maxFD = newSocket;

			char buff[64] = "The message";
			send(newSocket, buff, strlen(buff), 0);
		} else if (FD_ISSET(clientSocket, &readSet)) {
			//TODO: request from client
			struct sockaddr_in from;
			socklen_t fromlen = sizeof(from);
			std::cout << "Receiving message from client" << std::endl;
			char buff[64] = "";
			if (SOCKET_ERROR == recvfrom(clientSocket, buff, sizeof(buff), 0, (struct sockaddr*)&from, &fromlen)) {
				std::cout << "Failed to receive client packet" << std::endl;
				continue;
			}

			char fromIP[16] = "";
			inet_ntop(from.sin_family, &from.sin_addr.s_addr, fromIP, sizeof(fromIP));
			printf("Message received: %s, From IP: %s, Port: %u\n", buff, fromIP, from.sin_port);

			from.sin_port = RESPONSE_TX_PORT;
			int bytesWrote = sendto(clientSocket, buff, strlen(buff), 0, (struct sockaddr *)&from, fromlen);
			if (SOCKET_ERROR == bytesWrote) {
				std::cout << "Failed to send client packet" << std::endl;
				continue;
			}
			printf("Bytes wrote %d\n", bytesWrote);

			//TODO: determine which server to send to


			//TODO: send to selected server

		} else {
			std::cout << "Receiving messages from servers" << std::endl;
			for (const auto &sp : serverSockets) {
				if (FD_ISSET(sp.first, &readSet)) {
					//TODO: response from servers
					char buff[64] = "";
					int bytesRead = recv(sp.first, buff, sizeof(buff), 0);
					if (-1 == bytesRead) {
						std::cout << "Failed to read from a server" << std::endl;
					} else if (0 == bytesRead) {
						//TODO: resolve disconnected server
						std::cout << "Server disconnected" << std::endl;
					} else {
						std::cout << "Server message: " << buff << std::endl;
					}

				}
			}
		}
	}



	closeSocket(serverSocket);
	closeSocket(clientSocket);
	for (const auto &sp : serverSockets) {
		closeSocket(sp.first);
	}

	cleanup();

	return 0;
}

