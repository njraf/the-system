#pragma once

#include <string>

constexpr size_t MTU = 1500;
constexpr size_t HEADER_SIZE = 28;
constexpr size_t MAX_STR_LEN = 64;

typedef struct {
	char clientIP[16];
	char packetType[4];
	uint32_t sessionID;
	uint32_t crc;
} PacketHeader;

typedef struct {
	uint32_t succcess;
	char message[MAX_STR_LEN];
} ResultPacket;

typedef struct {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
} SignInPacket;

typedef struct {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
	char firstName[MAX_STR_LEN];
	char lastName[MAX_STR_LEN];
} SignUpPacket;



// unpack //

void readPacketHeader(uint8_t *buff, PacketHeader &header);
void readSignInPacket(uint8_t *buff, SignInPacket &packet);
void readSignUpPacket(uint8_t *buff, SignUpPacket &packet);


// pack //

// call this after other pack functions so crc can create a checksum properly
void packHeader(uint8_t *buff, const PacketHeader &header);
void packResultPacket(uint8_t *buff, const ResultPacket &packet);




