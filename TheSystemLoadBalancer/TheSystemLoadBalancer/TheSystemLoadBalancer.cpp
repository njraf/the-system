
#include <iostream>
#include <vector>
#include <thread>
#include <utility> // std::pair
#include <signal.h>
#include <fstream>
#include <string>
#include <algorithm>

#include "sockets.h"
#include "ServerConnection.h"

volatile bool isRunning = true;
std::vector<ServerConnection*> serverSockets;

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
	for (const auto *s : serverSockets) {
		closeSocket(s->getSocket());
		delete s;
		s = nullptr;
	}
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
			}
			std::cout << "New server accepted" << std::endl;

			serverSockets.push_back(new ServerConnection(newSocket, theirAddr, theirSize));
			FD_SET(newSocket, &mainSet);
			maxFD = newSocket;
		} else if (FD_ISSET(clientSocket, &readSet)) {
			// request from client
			std::cout << "Receiving message from client" << std::endl;
			struct sockaddr_in from;
			socklen_t fromlen = sizeof(from);
			
			constexpr int PACKET_SIZE = 64;
			uint8_t buff[PACKET_SIZE];
			memset(buff, 0, sizeof(buff));
			if (SOCKET_ERROR == recvfrom(clientSocket, (char*)buff, sizeof(buff), 0, (struct sockaddr*)&from, &fromlen)) {
				std::cout << "Failed to receive client packet" << std::endl;
				continue;
			}

			// determine which server to send to
			if (serverSockets.empty()) {
				std::cout << "Received client request, but there are no active servers to send to" << std::endl;
				continue;
			}

			const auto minIt = std::min_element(serverSockets.begin(), serverSockets.end(), [=](ServerConnection *s1, ServerConnection *s2) { return (s1->getActiveRequests() < s2->getActiveRequests()); });
			if (minIt == serverSockets.end()) {
				std::cout << "Error: Could not find a server to send to" << std::endl;
			}

			ServerConnection *selectedServer = (*minIt);
			
			if (!selectedServer->sendPacket(buff, sizeof(buff))) {
				std::cout << "Warning: Failed to send to server" << std::endl;
			}
		} else {
			std::cout << "Receiving messages from servers" << std::endl;
			for (const auto *s : serverSockets) {
				if (FD_ISSET(s->getSocket(), &readSet)) {
					// response from servers
					constexpr int PACKET_SIZE = 64;
					uint8_t buff[PACKET_SIZE];
					memset(buff, 0, sizeof(buff));
					s->recvPacket(buff, sizeof(buff));

					// send response to client

					char clientIP[16] = "";
					memcpy(clientIP, buff, 16);

					struct sockaddr_in addr;
					switch (makeSockaddr(addr, AF_INET, clientIP, RESPONSE_TX_PORT)) {
					case -1:
						std::cout << "Failed to make sockaddr_in for the client socket" << std::endl;
						continue;
					case 0:
						std::cout << "Invalid IP address" << std::endl;
						continue;
					case 1:
					default:
						break;
					}

					int bytesWrote = sendto(clientSocket, (char*)buff, sizeof(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
					if (SOCKET_ERROR == bytesWrote) {
						std::cout << "Failed to send client packet" << std::endl;
						continue;
					}
					printf("Bytes wrote %d to IP %s\n", bytesWrote, clientIP);

				}
			}
		}
	}



	closeSocket(serverSocket);
	closeSocket(clientSocket);
	
	cleanup();

	return 0;
}

