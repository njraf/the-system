/*

threads:
- one per request to do socket I/O and DB access


sockets:
- one for the load balancer I/O

*/
#include <iostream>
#include <string>

#include "sockets.h"

const std::string LOAD_BALANCER_IP = "127.0.0.1";
const int LOAD_BALANCER_PORT = 3576;
volatile bool isRunning = true;

int main() {
    
    // connect to load balancer over TCP
    SOCKET sock = createSocket(AF_INET, SOCK_STREAM, 0);
    if (!isValidSocket(sock)) {
        std::cout << "Failed to create the socket" << std::endl;
        return 1;
    }

    struct sockaddr_in addr;
    switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), LOAD_BALANCER_PORT)) {
    case -1:
        std::cout << "Failed to make sockaddr_in" << std::endl;
        closeSocket(sock);
        return 1;
    case 0:
        std::cout << "Invalid IP address" << std::endl;
        return 1;
    case 1:
    default:
        break;
    }

    if (SOCKET_ERROR == connect(sock, (struct sockaddr *)&addr, sizeof(addr))) {
        std::cout << "Failed to connect to the Load Balancer" << std::endl;
        closeSocket(sock);
        return 1;
    }

    while (isRunning) {
        // wait for requests
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int ret = select(sock + 1, &readSet, nullptr, nullptr, &tv);
        if (-1 == ret) {
            std::cout << "Select error" << std::endl;
            continue;
        } else if (0 == ret) {
            std::cout << "Select timeout" << std::endl;
            continue;
        } else if (!FD_ISSET(sock, &readSet)) {
            std::cout << "Socket not selected" << std::endl;
            continue;
        }

        // read packets


        // parse packets


        // execute request

    }



    closeSocket(sock);
}

