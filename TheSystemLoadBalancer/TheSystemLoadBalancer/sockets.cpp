
#include "sockets.h"

SOCKET createSocket(int domain, int type, int protocol) {
#if defined(_WIN32)
	WSADATA d;
	static bool winsockInitialized = false;
	if (!winsockInitialized) {
		if (WSAStartup(MAKEWORD(2, 2), &d)) {
			std::cout << "Failed to initialize WSA" << std::endl;
			return -1;
		} else {
			winsockInitialized = true;
		}
	}
#endif

	return socket(domain, type, protocol);
}

void closeSocket(SOCKET sock) {
#if defined(_WIN32)
	closesocket(sock);
#else
	close(sock);
#endif
}

int getSocketErrno() {
#if defined(_WIN32)
	return WSAGetLastError();
#else
	return errno;
#endif
}

void printErrorText(int error) {
#if defined(_WIN32)
	LPWSTR message = nullptr;
	const DWORD MESSAGE_BUFFER_SIZE = 256;
	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, error, 0, message, MESSAGE_BUFFER_SIZE, 0);

	printf("%S\n", message);
#else

	printf("%s\n", strerror(error));

#endif
}

bool isValidSocket(SOCKET sock) {
	return (INVALID_SOCKET != sock);
}

int makeSockaddr(struct sockaddr_in &addr, ADDRESS_FAMILY family, const char *address, USHORT port) {
	addr.sin_family = family;
	addr.sin_port = port;
	return inet_pton(family, address, &(addr.sin_addr.s_addr));
}
