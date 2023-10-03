#include "PacketFormats.h"
#include "sockets.h"

void readPacketHeader(uint8_t *buff, PacketHeader &header) {
    // check packet type
    uint8_t *buffPtr = buff;
    memcpy(header.clientIP, buffPtr, sizeof(header.clientIP));
    buffPtr += 16;
    memcpy(header.packetType, buffPtr, sizeof(uint32_t));
    buffPtr += sizeof(uint32_t);
    header.sessionID = ntohl(*buffPtr);
    buffPtr += 4;
    header.crc = ntohl(*buffPtr);
}

void readSigninPacket(uint8_t *buff, SignInPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    memcpy(packet.username, buffPtr, sizeof(packet.username));
    buffPtr += sizeof(packet.username);
    memcpy(packet.password, buffPtr, sizeof(packet.password));
    buffPtr += sizeof(packet.password);
}