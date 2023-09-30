#pragma once

#include "sockets.h"

class ServerConnection {

public:
	ServerConnection(SOCKET sock_, struct sockaddr_storage addrStorage_, socklen_t sockLen_);
	~ServerConnection() = default;

	SOCKET getSocket() const;
	struct sockaddr_storage getAddressStorage() const;
	socklen_t getAddressLength() const;

private:
	SOCKET sock;
	struct sockaddr_storage addrStorage;
	socklen_t sockLen;

};

