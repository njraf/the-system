
#include <iostream>
#include <vector>
#include <thread>
#include <utility> // std::pair
#include <signal.h>

#include "sockets.h"

volatile bool isRunning = true;
std::vector<std::pair<SOCKET, struct sockaddr_storage>> serverSockets;

void signalHandler(int signal) {
	std::cout << "Handling signal" << std::endl;
	isRunning = false;
}

int main() {

	if (SIG_ERR == signal(SIGINT, signalHandler)) {
		std::cout << "Failed to set signal handler" << std::endl;
		return 1;
	}

	const std::string LOAD_BALANCER_IP = "127.0.0.1";
	const int LOAD_BALANCER_PORT = 3576;

	// connect to load balancer over TCP
	SOCKET sock = createSocket(AF_INET, SOCK_STREAM, 0);
	if (!isValidSocket(sock)) {
		std::cout << "Failed to create the socket" << std::endl;
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	}

	struct sockaddr_in addr;
	switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), LOAD_BALANCER_PORT)) {
	case -1:
		std::cout << "Failed to make sockaddr_in" << std::endl;
		closeSocket(sock);
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	case 0:
		std::cout << "Invalid IP address" << std::endl;
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	case 1:
	default:
		break;
	}

	if (SOCKET_ERROR == bind(sock, (struct sockaddr *)&addr, sizeof(addr))) {
		std::cout << "Failed to bind the socket" << std::endl;
		closeSocket(sock);
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	}

	if (SOCKET_ERROR == listen(sock, 10)) {
		std::cout << "Failed to listen on the socket" << std::endl;
		closeSocket(sock);
#if defined(_WIN32)
		WSACleanup();
#endif
		return 1;
	}


	while (isRunning) {
		struct sockaddr_storage theirAddr;
		int theirSize = sizeof(theirAddr);

		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(sock, &readSet);

		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		int ret = select(sock + 1, &readSet, nullptr, nullptr, &tv);
		if (-1 == ret) {
			// error
			std::cout << "Select error" << std::endl;
			continue;
		} else if (0 == ret) {
			// timeout
			continue;
		} else if (!FD_ISSET(sock, &readSet)) {
			std::cout << "Socket not selected" << std::endl;
			continue;
		}

		// wait for requests
		SOCKET newSocket = accept(sock, (struct sockaddr *)&theirAddr, &theirSize);
		if (INVALID_SOCKET == newSocket) {
			std::cout << "Client accept failed" << std::endl;
			continue;
		}

		serverSockets.push_back(std::make_pair(newSocket, theirAddr));



	}



	closeSocket(sock);
	for (const auto &sp : serverSockets) {
		closesocket(sp.first);
	}

#if defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

