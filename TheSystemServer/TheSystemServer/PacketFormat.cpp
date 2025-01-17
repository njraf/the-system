#include "PacketFormats.h"
#include "sockets.h"

#include "zlib.h"

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

void unpackHeader(uint8_t *buff, size_t packetSize_, PacketHeader &header) {
    // check packet type
    uint8_t *buffPtr = buff;
    uint32_t val32 = 0;
    memcpy(&val32, buffPtr, sizeof(header.crc));
    header.crc = ntohl(val32);
    buffPtr += sizeof(header.crc);
    memcpy(header.clientIP, buffPtr, sizeof(header.clientIP));
    buffPtr += sizeof(header.clientIP);
    memcpy(header.packetType, buffPtr, sizeof(header.packetType));
    buffPtr += sizeof(header.packetType);
    memcpy(&val32, buffPtr, sizeof(header.sessionID));
    header.sessionID = ntohl(val32);
    buffPtr += sizeof(header.sessionID);
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

void packHeader(uint8_t *buff, size_t packetSize_, const PacketHeader &header) {
    uint8_t *buffPtr = buff;
    buffPtr += sizeof(header.crc); // skip crc32
    memcpy(buffPtr, header.clientIP, sizeof(header.clientIP));
    buffPtr += sizeof(header.clientIP);
    memcpy(buffPtr, header.packetType, sizeof(header.packetType));
    buffPtr += sizeof(header.packetType);
    uint32_t val32 = htonl(header.sessionID);
    memcpy(buffPtr, &val32, sizeof(header.sessionID));
    buffPtr += sizeof(val32);

    const uint32_t CRC = crc32(0, (Bytef *)(buff + sizeof(uint32_t)), (uInt)(packetSize_ - sizeof(uint32_t)));
    val32 = htonl(CRC);
    memcpy(buff, &val32, sizeof(val32));
}

void packResultPacket(uint8_t *buff, const ResultPacket &packet) {
    uint8_t *buffPtr = buff + sizeof(PacketHeader);
    uint32_t val32 = htonl(packet.success);
    memcpy(buffPtr, &val32, sizeof(packet.success));
    buffPtr += sizeof(packet.success);
    memcpy(buffPtr, packet.message, sizeof(packet.message));
}
