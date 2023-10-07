#pragma once

#include <string>

constexpr size_t MTU = 1500;
constexpr size_t HEADER_SIZE = 28;
constexpr size_t MAX_STR_LEN = 64;

typedef struct {
	char clientIP[16];
	char packetType[5];
	uint32_t sessionID;
	uint32_t crc;
} PacketHeader;

typedef struct {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
} SignInPacket;

typedef struct {
	uint32_t succcess;
	char message[MAX_STR_LEN];
} ResultPacket;

// unpack //

void readPacketHeader(uint8_t *buff, PacketHeader &header);
void readSigninPacket(uint8_t *buff, SignInPacket &packet);


// pack //

// call this after other pack functions so crc can create a checksum properly
void packHeader(uint8_t *buff, const PacketHeader &header);
void packResultPacket(uint8_t *buff, const ResultPacket &packet);




