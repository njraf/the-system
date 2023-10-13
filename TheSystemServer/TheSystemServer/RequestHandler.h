#pragma once
#include <stdint.h>
#include <string>

#include "sockets.h"
#include "DatabaseManager.h"
#include "PacketFormats.h"

class RequestHandler {
public:
	RequestHandler(std::shared_ptr<DatabaseManager> databaseManager_);
	~RequestHandler() = default;

	bool verifyHeader(uint8_t *buff, std::string &packetType);
	void resolveSignIn(uint8_t *buff, SOCKET sock);
	void resolveSignUp(uint8_t *buff, SOCKET sock);


private:
	const int PACKET_HEADER_SIZE = 28;
	std::shared_ptr<DatabaseManager> databaseManager;
};

