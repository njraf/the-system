#include "RequestHandler.h"
#include "SignInDAO.h"

#include <iostream>

RequestHandler::RequestHandler(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool RequestHandler::verifyHeader(uint8_t *buff, std::string &packetType) {
	PacketHeader header;
	readPacketHeader(buff, header);

	// check packet type
	packetType = header.packetType;
	std::cout << "Receiving message with type: " << packetType << std::endl;

	//TODO: check session ID
	//if (0 < header.sessionID) {
	// check database for the session ID
	//}
	

	//TODO: check CRC


	return true;
}

void RequestHandler::resolveSignIn(uint8_t *buff, SOCKET sock) {
	PacketHeader header;
	readPacketHeader(buff, header);

	SignInPacket packet;
	readSigninPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);

	ResultPacket resultPacket;

	// check if username exists in the database
	SignInDAO signInDAO(databaseManager);

	// generate new session ID
	if (0 == header.sessionID) {
		//TODO: generate a new session ID
	}

	//TODO: temp for testing
	resultPacket.succcess = true;
	std::string msg = "Hello Message";
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());

	// create success/fail result message
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(responseBuff), 0);
}
