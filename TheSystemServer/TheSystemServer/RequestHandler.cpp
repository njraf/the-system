#include "RequestHandler.h"
#include "UsersDAO.h"

#include <iostream>

RequestHandler::RequestHandler(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool RequestHandler::verifyHeader(uint8_t *buff, std::string &packetType) {
	PacketHeader header;
	unpackPacketHeader(buff, header);

	// check packet type
	packetType = header.packetType;
	std::cout << "Receiving message with type: " << header.packetType << std::endl;

	//TODO: check CRC
	
	
	//TODO: check session ID
	if (0 < header.sessionID) {

	}


	return true;
}

void RequestHandler::resolveSignIn(uint8_t *buff, socket_t sock) {
	PacketHeader header;
	unpackPacketHeader(buff, header);

	SignInPacket packet;
	unpackSignInPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);

	ResultPacket resultPacket{};

	// check if username exists in the database
	UsersDAO usersDAO(databaseManager);
	resultPacket.success = static_cast<uint32_t>(usersDAO.isValidSignInAttempt(username, password));

	// generate new session ID
	if (0 == header.sessionID) {
		//TODO: generate a new session ID
	}
	memcpy(header.packetType, "RSLT", sizeof(header.packetType));

	// set result message
	std::string msg = resultPacket.success ? "Please wait while we sign you in" : "Username or password were not recognized";
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());

	// create success/fail result packet
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(responseBuff), 0);
}

void RequestHandler::resolveSignUp(uint8_t *buff, socket_t sock) {
	PacketHeader header;
	unpackPacketHeader(buff, header);

	SignUpPacket packet;
	unpackSignUpPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);
	std::string firstName(packet.firstName);
	std::string lastName(packet.lastName);

	UsersDAO usersDAO(databaseManager);
	ResultPacket resultPacket{};
	std::string msg = "";

	if (usersDAO.userExists(username)) {
		// user already exists
		resultPacket.success = 0;
		msg = "This username is already taken";
	} else if (usersDAO.createUser(firstName, lastName, username, password)) {
		// user does not exist and new user was created successfully
		resultPacket.success = 1;
		msg = "User was created successfully";
	} else { // failed to create user
		resultPacket.success = 0;
		msg = "Failed to create a new account";
	}
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());

	// generate new session ID
	if (0 == header.sessionID) {
		//TODO: generate a new session ID
	}
	memcpy(header.packetType, "RSLT", sizeof(header.packetType));

	// create success/fail result packet
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(responseBuff), 0);
}
