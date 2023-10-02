#include "RequestHandler.h"
#include <iostream>

bool RequestHandler::verifyHeader(uint8_t *buff, std::string &packetType) {
    // check packet type
    char cPacketType[5] = "";
    uint8_t *buffPtr = buff + 16;
    memcpy(cPacketType, buffPtr, 4);
    packetType.assign(cPacketType);
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
    std::string username = cUsername;
    std::string password = cPassword;

    // check if username exists in the database


    // create new user un the database


    // generate new session ID


    // create success/fail result message


    // send response
    send(sock, (char *)buff, sizeof(buff), 0);
}
