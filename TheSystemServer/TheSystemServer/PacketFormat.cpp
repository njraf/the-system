#include "PacketFormats.h"
#include "sockets.h"

// unpack //

void readPacketHeader(uint8_t *buff, PacketHeader &header) {
    // check packet type
    uint8_t *buffPtr = buff;
    memcpy(header.clientIP, buffPtr, sizeof(header.clientIP));
    buffPtr += 16;
    memcpy(header.packetType, buffPtr, sizeof(uint32_t));
    header.packetType[4] = '\0';
    buffPtr += sizeof(uint32_t);
    header.sessionID = ntohl(*buffPtr);
    buffPtr += 4;
    header.crc = ntohl(*buffPtr);
}

void readSignInPacket(uint8_t *buff, SignInPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    memcpy(packet.username, buffPtr, sizeof(packet.username));
    buffPtr += sizeof(packet.username);
    memcpy(packet.password, buffPtr, sizeof(packet.password));
    buffPtr += sizeof(packet.password);
    std::cout << "username " << packet.username << " password " << packet.password << std::endl;
}

void readSignUpPacket(uint8_t *buff, SignUpPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    memcpy(packet.username, buffPtr, sizeof(packet.username));
    buffPtr += sizeof(packet.username);
    memcpy(packet.password, buffPtr, sizeof(packet.password));
    buffPtr += sizeof(packet.password);
    memcpy(packet.firstName, buffPtr, sizeof(packet.firstName));
    buffPtr += sizeof(packet.firstName);
    memcpy(packet.lastName, buffPtr, sizeof(packet.lastName));
    buffPtr += sizeof(packet.lastName);
}


// pack //

void packHeader(uint8_t *buff, const PacketHeader &header) {
    uint8_t *buffPtr = buff;
    memcpy(buffPtr, header.clientIP, sizeof(header.clientIP));
    buffPtr += 16;
    memcpy(buffPtr, header.packetType, sizeof(uint32_t));
    buffPtr += sizeof(uint32_t);
    *buffPtr = htonl(header.sessionID);
    buffPtr += sizeof(header.sessionID);
    *buffPtr = htonl(header.crc);
}

void packResultPacket(uint8_t *buff, const ResultPacket &packet) {
    uint8_t *buffPtr = buff + HEADER_SIZE;
    *buffPtr = htonl(packet.succcess);
    buffPtr += sizeof(packet.succcess);
    memcpy(buffPtr, packet.message, sizeof(packet.message));
}
