/*

threads:
- one per request to do socket I/O and DB access


sockets:
- one for the load balancer I/O

*/
#include <iostream>
#include <string>
#include <fstream>
#include <signal.h>

#include "sockets.h"
#include "DatabaseManager.h"
#include "RequestHandler.h"

const long SELECT_TIMEOUT_SEC = 1;
std::string DATABASE_IP = "127.0.0.1";
std::string LOAD_BALANCER_IP = "127.0.0.1";
std::string SERVER_IP = "127.0.0.1";
const int REQUEST_RX_PORT = 3578; // from load balancer
const int RESPONSE_TX_PORT = 3577; // to load balancer
volatile bool isRunning = true;

std::shared_ptr<mysqlx::Session> session;

void signalHandler(int signal) {
    std::cout << "Handling signal. Wait for at most " << SELECT_TIMEOUT_SEC << " seconds" << std::endl;
    isRunning = false;
}

void cleanup() {
    isRunning = false;
#if defined(_WIN32)
    WSACleanup();
#endif
    session->close();
}

bool loadAddresses() {
    std::ifstream file;
    file.open("../../addresses.csv");
    if (!file.is_open()) {
        file.open("../../../addresses.csv");
        if (!file.is_open()) {
            return false;
        }
    }

    std::string line = "";
    const std::string delimiter = ",";
    while (std::getline(file, line)) {
        if (line.substr(0, line.find(delimiter)) == "load_balancer") {
            line.erase(0, line.find(delimiter) + delimiter.length()); // erase the first entry in the delimited string
            LOAD_BALANCER_IP = line.substr(0, line.find(delimiter));

            // trim whitespace from the right (especially CRLF)
            LOAD_BALANCER_IP.erase(std::find_if(LOAD_BALANCER_IP.rbegin(), LOAD_BALANCER_IP.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
                }).base(), LOAD_BALANCER_IP.end());
            std::cout << "Setting own IP address to " << LOAD_BALANCER_IP << std::endl;
            break;
        } else if (line.substr(0, line.find(delimiter)) == "server") {
            line.erase(0, line.find(delimiter) + delimiter.length()); // erase the first entry in the delimited string
            LOAD_BALANCER_IP = line.substr(0, line.find(delimiter));

            // trim whitespace from the right (especially CRLF)
            SERVER_IP.erase(std::find_if(SERVER_IP.rbegin(), SERVER_IP.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
                }).base(), SERVER_IP.end());
            std::cout << "Setting own IP address to " << SERVER_IP << std::endl;
            break;
        }
    }

    file.close();
    return true;
}

int main() {

    if (SIG_ERR == signal(SIGINT, signalHandler)) {
        std::cout << "Failed to set signal handler" << std::endl;
        return 1;
    }

    if (!loadAddresses()) {
        std::cout << "Failed to set IP addresses" << std::endl;
        return 1;
    }

    // read database username and password from a file
    constexpr int FILE_BUFFER_SIZE = 64;
    char dbUsername[FILE_BUFFER_SIZE];
    char dbPassword[FILE_BUFFER_SIZE];
    std::ifstream file;

    file.open("credentials.txt", std::ifstream::in);
    if (!file.is_open()) {
        std::cout << "Could not open the credentials file" << std::endl;
        cleanup();
        return 1;
    }

    file.getline(dbUsername, FILE_BUFFER_SIZE - 1);
    file.getline(dbPassword, FILE_BUFFER_SIZE - 1);
    file.close();


    // connect to the database
    try {
        std::cout << "Connecting to database" << std::endl;
        const int DB_PORT = 33060;
        session = std::make_shared<mysqlx::Session>(DATABASE_IP, DB_PORT, std::string(dbUsername), std::string(dbPassword));
        session->sql(mysqlx::string("USE ") + mysqlx::string("the_system") + mysqlx::string(";")).execute();
    } catch (std::exception e) {
        std::cout << "Error: Could not connect to database. " << e.what() << std::endl;
        cleanup();
        return 1;
    }
    memset(dbUsername, 0, FILE_BUFFER_SIZE);
    memset(dbPassword, 0, FILE_BUFFER_SIZE);
    
    std::shared_ptr<DatabaseManager> databaseManager = std::make_shared<DatabaseManager>(session, "the_system");

    // connect to load balancer over TCP
    socket_t sock = createSocket(AF_INET, SOCK_STREAM, 0);
    if (!isValidSocket(sock)) {
        std::cout << "Failed to create the socket" << std::endl;
        cleanup();
        return 1;
    }

    struct sockaddr_in addr;
    switch (makeSockaddr(addr, AF_INET, LOAD_BALANCER_IP.c_str(), RESPONSE_TX_PORT)) {
    case -1:
        std::cout << "Failed to make sockaddr_in" << std::endl;
        closeSocket(sock);
        cleanup();
        return 1;
    case 0:
        std::cout << "Invalid IP address" << std::endl;
        closeSocket(sock);
        cleanup();
        return 1;
    case 1:
    default:
        break;
    }

    if (SOCKET_ERROR == connect(sock, (struct sockaddr *)&addr, sizeof(addr))) {
        std::cout << "Failed to connect to the Load Balancer" << std::endl;
        closeSocket(sock);
        cleanup();
        return 1;
    } else {
        std::cout << "Connected to load balancer" << std::endl;
    }

    RequestHandler requestHandler(databaseManager);

    while (isRunning) {
        // wait for requests
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);

        struct timeval tv;
        tv.tv_sec = SELECT_TIMEOUT_SEC;
        tv.tv_usec = 0;

        int ret = select(sock + 1, &readSet, nullptr, nullptr, &tv);
        if (-1 == ret) {
            std::cout << "Select error" << std::endl;
            errno = 0;
            continue;
        } else if (0 == ret) {
            //std::cout << "Select timeout" << std::endl;
            continue;
        } else if (!FD_ISSET(sock, &readSet)) {
            std::cout << "Socket not selected" << std::endl;
            continue;
        }

        // read packets
        uint8_t buff[MTU];
        memset(buff, 0, sizeof(buff));
        int bytesRead = recv(sock, (char*)buff, sizeof(buff), 0);
        if (SOCKET_ERROR == bytesRead) {
            std::cout << "Read failed" << std::endl;
            errno = 0;
        } else if (0 == bytesRead) {
            std::cout << "Load balancer disconnected" << std::endl;
            break;
        } else if (!FD_ISSET(sock, &readSet)) {
            continue;
        }

        
        

        // parse packets
        std::string packetType = "";
        if (!requestHandler.verifyHeader(buff, bytesRead, packetType)) { //TODO: can bytesRead be more than one packet?
            std::cout << "Invalid packet header" << std::endl;

            PacketHeader header;
            unpackHeader(buff, 0, header);
            strncpy_s(header.packetType, 4, "RSLT", 4);

            ResultPacket resultPacket;
            std::string msg = "ERROR: Bad packet";
            strncpy_s(resultPacket.message, sizeof(resultPacket.message), msg.c_str(), msg.length());
            resultPacket.success = false;

            uint8_t responseBuff[MTU];
            memset(responseBuff, 0, sizeof(responseBuff));
            packResultPacket(responseBuff, resultPacket);
            packHeader(responseBuff, header);
            send(sock, (char *)responseBuff, sizeof(PacketHeader) + sizeof(ResultPacket), 0);
            continue;
        }

        // execute request
        if ("SNIN" == packetType) {
            requestHandler.resolveSignIn(buff, sock);
        } else if ("SNUP" == packetType) {
            requestHandler.resolveSignUp(buff, sock);
        } else if ("" == packetType) {

        } else if ("" == packetType) {

        } else if ("" == packetType) {

        } else if ("" == packetType) {

        } else {
            std::cout << "Unrecognized packet type " << packetType << std::endl;
            continue;
        }
    }



    closeSocket(sock);
    cleanup();

    return 0;
}

