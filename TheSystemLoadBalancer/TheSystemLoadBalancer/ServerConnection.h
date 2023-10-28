#pragma once

#include "sockets.h"

class ServerConnection {

public:
	ServerConnection(socket_t sock_, struct sockaddr_storage addrStorage_, socklen_t sockLen_);
	~ServerConnection() = default;

	socket_t getSocket() const;
	struct sockaddr_storage getAddressStorage() const;
	socklen_t getAddressLength() const;
	int getActiveRequests() const;
	void invalidateSocket();
	
	bool sendPacket(uint8_t *buff, int len);
	int recvPacket(uint8_t *buff, int len) const;

private:
	socket_t sock;
	struct sockaddr_storage addrStorage;
	socklen_t sockLen;
	mutable int activeRequests;
};

