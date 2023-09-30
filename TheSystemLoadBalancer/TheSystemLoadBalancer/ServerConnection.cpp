#include "ServerConnection.h"

ServerConnection::ServerConnection(SOCKET sock_, struct sockaddr_storage addrStorage_, socklen_t sockLen_) 
	: sock(sock_)
	, addrStorage(addrStorage_)
	, sockLen(sockLen_)
{

}


SOCKET ServerConnection::getSocket() const {
	return sock;
}

struct sockaddr_storage ServerConnection::getAddressStorage() const {
	return addrStorage;
}

socklen_t ServerConnection::getAddressLength() const {
	return sockLen;
}