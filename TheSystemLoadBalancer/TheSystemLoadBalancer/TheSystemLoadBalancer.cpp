
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


	SOCKET serverSocket;
	if (!createServerSocket(serverSocket)) {
		closeSocket(serverSocket);
		cleanup();
		return 1;
	}


	SOCKET clientSocket;
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

			serverSockets.push_back(std::make_pair(newSocket, theirAddr));
			FD_SET(newSocket, &mainSet);
			maxFD = newSocket;
		} else if (FD_ISSET(clientSocket, &readSet)) {
			//TODO: request from client
			std::cout << "Receiving message from client" << std::endl;
			char buff[64] = "";
			if (SOCKET_ERROR == recv(clientSocket, buff, sizeof(buff), 0)) {
				std::cout << "Failed to receive client packet" << std::endl;
				continue;
			}
			printf("Message received: %s\n", buff);

			//TODO: determine which server to send to


			//TODO: send to selected server

		} else {
			std::cout << "Receiving messages from servers" << std::endl;
			for (const auto &sp : serverSockets) {
				if (FD_ISSET(sp.first, &readSet)) {
					//TODO: response from servers
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

