#include "PacketFormats.h"
#include "sockets.h"

void printHex(uint8_t *buff, size_t size) {
    for (int i = 0; i < size; i++) {
        if ((i % 16) == 0) {
            printf("%02X | ", i);
        }
        if ((i % 16) == 8) {
            printf("\t");
        }
        printf("%02X ", buff[i]);
        if ((i % 16) == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

// unpack //

void unpackPacketHeader(uint8_t *buff, PacketHeader &header) {
    // check packet type
    uint8_t *buffPtr = buff;
    memcpy(header.clientIP, buffPtr, sizeof(header.clientIP));
    buffPtr += 16;
    memcpy(header.packetType, buffPtr, sizeof(header.packetType));
    buffPtr += sizeof(header.packetType);
    header.sessionID = ntohl(*buffPtr);
    buffPtr += 4;
    header.crc = ntohl(*buffPtr);
}

void unpackSignInPacket(uint8_t *buff, SignInPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    memcpy(packet.username, buffPtr, sizeof(packet.username));
    buffPtr += sizeof(packet.username);
    memcpy(packet.password, buffPtr, sizeof(packet.password));
    buffPtr += sizeof(packet.password);
}

void unpackSignUpPacket(uint8_t *buff, SignUpPacket &packet) {
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
    memcpy(buffPtr, header.packetType, sizeof(header.packetType));
    buffPtr += sizeof(header.packetType);
    *buffPtr = htonl(header.sessionID);
    buffPtr += sizeof(header.sessionID);

    //TODO: do crc
    *buffPtr = htonl(header.crc);
}

void packResultPacket(uint8_t *buff, const ResultPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    uint32_t val32 = htonl(packet.success);
    memcpy(buffPtr, &val32, sizeof(val32));
    buffPtr += sizeof(packet.success);
    memcpy(buffPtr, packet.message, sizeof(packet.message));
}
