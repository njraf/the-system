#include "RequestHandler.h"
#include "UsersDAO.h"

#include <iostream>

RequestHandler::RequestHandler(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool RequestHandler::verifyHeader(uint8_t *buff, std::string &packetType) {
	PacketHeader header;
	readPacketHeader(buff, header);

	// check packet type
	packetType = header.packetType;
	std::cout << "Receiving message with type: " << header.packetType << std::endl;

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
	readSignInPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);

	ResultPacket resultPacket{};

	// check if username exists in the database
	UsersDAO usersDAO(databaseManager);
	resultPacket.succcess = usersDAO.isValidSignInAttempt(username, password);

	// generate new session ID
	if (0 == header.sessionID) {
		//TODO: generate a new session ID
	}
	strncpy_s(header.packetType, sizeof(header.packetType), "RSLT", 4);

	// set result message
	std::string msg = resultPacket.succcess ? "Please wait while we sign you in" : "Username or password were not recognized";
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length()); 

	// create success/fail result packet
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(responseBuff), 0);
}

void RequestHandler::resolveSignUp(uint8_t *buff, SOCKET sock) {
	PacketHeader header;
	readPacketHeader(buff, header);

	SignUpPacket packet;
	readSignUpPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);
	std::string firstName(packet.firstName);
	std::string lastName(packet.lastName);
}
