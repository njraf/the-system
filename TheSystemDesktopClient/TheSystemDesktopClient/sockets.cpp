
#include "sockets.h"

socket_t createSocket(int domain, int type, int protocol) {
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

void closeSocket(socket_t sock) {
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

void printErrorText() {
#if defined(_WIN32)
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%S\n", s);
	LocalFree(s);
#else
	printf("%s\n", strerror(errno));
#endif
}

bool isValidSocket(socket_t sock) {
	return (INVALID_SOCKET != sock);
}

int makeSockaddr(struct sockaddr_in &addr, ADDRESS_FAMILY family, const char *address, USHORT port) {
	addr.sin_family = family;
	addr.sin_port = port;
    //return inet_pton(family, address, &(addr.sin_addr.s_addr));

    addr.sin_addr.s_addr = inet_addr(address);
    return addr.sin_addr.s_addr;
}
