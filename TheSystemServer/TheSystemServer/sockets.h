#pragma once

#include <iostream>
#include <string.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#define SOCK socket_t
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define socket_t int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define USHORT unsigned short
#define ADDRESS_FAMILY unsigned short
#define strncpy_s(dest, s1, src, s2) strncpy(dest, src, s2)
#endif

socket_t createSocket(int domain, int type, int protocol);

void closeSocket(socket_t sock);

int getSocketErrno();

void printErrorText();

bool isValidSocket(socket_t sock);

int makeSockaddr(struct sockaddr_in &addr, ADDRESS_FAMILY family, const char *address, USHORT port);
