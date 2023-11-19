#include "ServerConnection.h"

ServerConnection::ServerConnection(socket_t sock_, struct sockaddr_storage addrStorage_, socklen_t sockLen_) 
	: sock(sock_)
	, addrStorage(addrStorage_)
	, sockLen(sockLen_)
	, activeRequests(0)
{

}


socket_t ServerConnection::getSocket() const {
	return sock;
}

struct sockaddr_storage ServerConnection::getAddressStorage() const {
	return addrStorage;
}

socklen_t ServerConnection::getAddressLength() const {
	return sockLen;
}

int ServerConnection::getActiveRequests() const {
	return activeRequests;
}

void ServerConnection::invalidateSocket() {
	sock = INVALID_SOCKET;
}

bool ServerConnection::sendPacket(uint8_t *buff, int len) {
	int bytesWrote = send(sock, (char*)buff, len, 0);
	if (SOCKET_ERROR == bytesWrote) {
		errno = 0;
		return false;
	}
	activeRequests++;
	std::cout << "Wrote " << bytesWrote <<" bytes" << std::endl;

	return true;
}

int ServerConnection::recvPacket(uint8_t *buff, int len) const {
	activeRequests--;
	int bytesRead = recv(sock, (char*)buff, len, 0);
	if (SOCKET_ERROR == bytesRead) {
		std::cout << "Failed to read from a server" << std::endl;
	} else if (0 == bytesRead) {
		std::cout << "Server disconnected" << std::endl;
		bytesRead = SOCKET_ERROR;
	} else {
		char message[64] = "";
		memcpy(message, buff + 32, 64);
		//std::cout << "Server message: " << message << std::endl;
	}
	return bytesRead;
}