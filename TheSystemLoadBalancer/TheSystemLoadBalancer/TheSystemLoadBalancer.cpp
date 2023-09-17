
#include <iostream>
#include <vector>
#include <thread>
#include <utility> // std::pair
#include <signal.h>

#include "sockets.h"

volatile bool isRunning = true;
std::vector<std::pair<SOCKET, struct sockaddr_storage>> serverSockets;

const long SELECT_TIMEOUT_SEC = 5;
const std::string LOAD_BALANCER_IP = "127.0.0.1";
const int SERVER_PORT = 3576;
const int CLIENT_PORT = 3577;

void signalHandler(int signal) {
	std::cout << "Handling signal. Wait for at most " << SELECT_TIMEOUT_SEC << " seconds" << std::endl;
	isRunning = false;
}

bool createServerSocket(SOCKET &serverSocket) {
	// listen for servers trying to connect over TCP
	serverSocket = createSocket(AF_INET, SOCK_STREAM, 0);
	if (!isValidSocket(serverSocket)) {
		std::cout << "Failed to create the server socket" << std::endl;
		return false;
	}

	struct sockaddr_in addr;
	switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), SERVER_PORT)) {
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
	switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), CLIENT_PORT)) {
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


	SOCKET serverSocket;
	if (!createServerSocket(serverSocket)) {
		closeSocket(serverSocket);
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	}


	SOCKET clientSocket;
	if (!createClientSocket(clientSocket)) {
		closeSocket(serverSocket);
		closeSocket(clientSocket);
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	}

	SOCKET maxFD = clientSocket;

	while (isRunning) {
		struct sockaddr_storage theirAddr;
		int theirSize = sizeof(theirAddr);

		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverSocket, &readSet);
		for (const auto &sp : serverSockets) {
			FD_SET(sp.first, &readSet);
		}

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
			SOCKET newSocket = accept(serverSocket, (struct sockaddr *)&theirAddr, &theirSize);
			if (INVALID_SOCKET == newSocket) {
				std::cout << "Client accept failed" << std::endl;
				continue;
			}

			serverSockets.push_back(std::make_pair(newSocket, theirAddr));
			maxFD = newSocket;
		} else if (FD_ISSET(clientSocket, &readSet)) {
			//TODO
		} else {
			for (const auto &sp : serverSockets) {
				if (FD_ISSET(sp.first, &readSet)) {
					//TODO
				}
			}
		}

		


	}



	closeSocket(serverSocket);
	closeSocket(clientSocket);
	for (const auto &sp : serverSockets) {
		closesocket(sp.first);
	}

#if defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

