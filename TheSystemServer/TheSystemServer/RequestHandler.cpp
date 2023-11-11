#include "RequestHandler.h"
#include "UsersDAO.h"
#include "SessionsDAO.h"

#include <iostream>

#include "zlib.h"

RequestHandler::RequestHandler(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool RequestHandler::verifyHeader(uint8_t *buff, size_t packetSize_, std::string &packetType) {
	PacketHeader header;
	unpackHeader(buff, packetSize_, header);

	// check packet type
	packetType = header.packetType;
	std::cout << "Receiving message with type: " << header.packetType << std::endl;

	// check CRC
	const uint32_t CRC = crc32(0, (Bytef*)(buff + sizeof(uint32_t)), packetSize_ - sizeof(uint32_t));
	if (CRC != header.crc) {
		std::cout << "ERROR: bad CRC" << std::endl;
		return false;
	}
	
	// check session ID
	if (0 < header.sessionID) {
		SessionsDAO sessionsDao(databaseManager);
		if (!sessionsDao.sessionExists(header.sessionID)) {
			std::cout << "ERROR: bad session ID" << std::endl;
			return false;
		}
	}

	return true;
}

void RequestHandler::resolveSignIn(uint8_t *buff, socket_t sock) {
	PacketHeader header;
	unpackHeader(buff, sizeof(PacketHeader) + sizeof(SignInPacket), header);

	SignInPacket packet;
	unpackSignInPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);

	ResultPacket resultPacket{};

	// check if username exists in the database
	UsersDAO usersDAO(databaseManager);
	bool success = usersDAO.isValidSignInAttempt(username, password);
	std::string msg = success ? "Please wait while we sign you in" : "Username or password were not recognized";

	// generate new session ID
	if (success && (0 == header.sessionID)) {
		SessionsDAO sessionsDao(databaseManager);
		uint32_t newSessionID = sessionsDao.createSession(username);
		success = (0 != newSessionID);
		if (success) {
			header.sessionID = newSessionID;
		}
		msg = success ? "Please wait while we sign you in" : "Could not create a session";
	}
	resultPacket.success = static_cast<uint32_t>(success);
	memcpy(header.packetType, "RSLT", sizeof(header.packetType));

	// set result message
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());

	// create success/fail result packet
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(PacketHeader) + sizeof(ResultPacket), 0);
}

void RequestHandler::resolveSignUp(uint8_t *buff, socket_t sock) {
	PacketHeader header;
	unpackHeader(buff, sizeof(PacketHeader) + sizeof(SignUpPacket), header);
	
	SignUpPacket packet;
	unpackSignUpPacket(buff, packet);

	std::string username(packet.username);
	std::string password(packet.password);
	std::string firstName(packet.firstName);
	std::string lastName(packet.lastName);

	UsersDAO usersDAO(databaseManager);
	ResultPacket resultPacket{};
	std::string msg = "";
	bool success = false;

	if (usersDAO.userExists(username)) {
		// user already exists
		success = false;
		msg = "This username is already taken";
	} else if (usersDAO.createUser(firstName, lastName, username, password)) {
		// user does not exist and new user was created successfully
		success = true;
		msg = "User was created successfully";
	} else { // failed to create user
		success = false;
		msg = "Failed to create a new account";
	}

	// generate new session ID
	if (success && (0 == header.sessionID)) {
		SessionsDAO sessionsDao(databaseManager);
		uint32_t newSessionID = sessionsDao.createSession(username);
		success = (0 != newSessionID);
		if (success) {
			header.sessionID = newSessionID;
		}
		msg = success ? "Please wait while we sign you in" : "Could not create a session";
	}
	memcpy(header.packetType, "RSLT", sizeof(header.packetType));
	resultPacket.success = static_cast<uint32_t>(success);
	strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());

	// create success/fail result packet
	uint8_t responseBuff[MTU];
	memset(responseBuff, 0, sizeof(responseBuff));
	packResultPacket(responseBuff, resultPacket);
	packHeader(responseBuff, header);

	// send response
	send(sock, (char *)responseBuff, sizeof(PacketHeader) + sizeof(ResultPacket), 0);
}
