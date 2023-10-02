#pragma once
#include <stdint.h>
#include <string>

#include "sockets.h"

class RequestHandler {
public:
	RequestHandler() = default;
	~RequestHandler() = default;

	bool verifyHeader(uint8_t *buff, std::string &packetType);
	void resolveSignIn(uint8_t *buff, SOCKET sock);


private:
	const int PACKET_HEADER_SIZE = 28;
};

