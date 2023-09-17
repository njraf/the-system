
#include <iostream>
#include <vector>
#include <thread>
#include <utility> // std::pair

#include "sockets.h"

volatile bool isRunning = true;
std::vector<std::pair<SOCKET, struct sockaddr_storage>> serverSockets;

int main() {

	const std::string LOAD_BALANCER_IP = "127.0.0.1";
	const int LOAD_BALANCER_PORT = 4136;

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

		// wait for requests
		SOCKET newSocket = accept(sock, (struct sockaddr *)&theirAddr, &theirSize);
		if (INVALID_SOCKET == newSocket) {
			continue;
		}

		serverSockets.push_back(std::make_pair(newSocket, theirAddr));



	}



	closeSocket(sock);

#if defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

