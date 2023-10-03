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

    //TODO: check CRC


    return true;
}

void RequestHandler::resolveSignIn(uint8_t *buff, SOCKET sock) {
    char cUsername[64] = "";
    char cPassword[64] = "";
    uint8_t *buffPtr = buff + PACKET_HEADER_SIZE;
    memcpy(cUsername, buffPtr, 64);
    buffPtr += 64;
    memcpy(cPassword, buffPtr, 64);
    buffPtr += 64;

    SignInPacket packet;
    readSigninPacket(buff, packet);

    std::string username(packet.username);
    std::string password(packet.password);

    // check if username exists in the database
    SignInDAO signInDAO(databaseManager);

    // generate new session ID


    // create success/fail result message


    // send response
    send(sock, (char *)buff, sizeof(buff), 0);
}
