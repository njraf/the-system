#pragma once

#include <string>

typedef struct {
	char clientIP[64];
	char packetType[64];
	uint32_t sessionID;
	uint32_t crc;
} PacketHeader;

typedef struct {
	char username[64];
	char password[64];
} SignInPacket;

void readPacketHeader(uint8_t *buff, PacketHeader &header);
void readSigninPacket(uint8_t *buff, SignInPacket &packet);