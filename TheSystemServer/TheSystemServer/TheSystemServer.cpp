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

const long SELECT_TIMEOUT_SEC = 1;
std::string DATABASE_IP = "127.0.0.1";
std::string LOAD_BALANCER_IP = "127.0.0.1";
std::string SERVER_IP = "127.0.0.1";
const int REQUEST_RX_PORT = 3578; // from load balancer
const int RESPONSE_TX_PORT = 3577; // to load balancer
volatile bool isRunning = true;

void signalHandler(int signal) {
    std::cout << "Handling signal. Wait for at most " << SELECT_TIMEOUT_SEC << " seconds" << std::endl;
    isRunning = false;
}

void cleanup() {
    isRunning = false;
#if defined(_WIN32)
    WSACleanup();
#endif
}

bool loadAddresses() {
    std::ifstream file;
    file.open("../../addresses.csv");
    if (!file.is_open()) {
        return false;
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
    std::shared_ptr<DatabaseManager> dbm;
    try {
        const int DB_PORT = 33060;
        dbm = std::make_shared<DatabaseManager>(DATABASE_IP, DB_PORT, std::string(dbUsername), std::string(dbPassword), "the_system");
    } catch (std::exception e) {
        std::cout << "Error: " << e.what() << std::endl;
        cleanup();
        return 1;
    }
    memset(dbUsername, 0, FILE_BUFFER_SIZE);
    memset(dbPassword, 0, FILE_BUFFER_SIZE);
    
    for (std::string asd : dbm->getSchema().getTableNames()) {
        // print table names
        //std::cout << asd << std::endl;
    }
    mysqlx::Table users = dbm->getSchema().getTable("users");
    dbm->printTable(users);
    

    // connect to load balancer over TCP
    SOCKET sock = createSocket(AF_INET, SOCK_STREAM, 0);
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
        char buff[64] = "";
        int bytesRead = recv(sock, buff, sizeof(buff), 0);
        if (-1 == bytesRead) {
            std::cout << "Read failed" << std::endl;
        } else if (0 == bytesRead) {
            std::cout << "Load balancer disconnected" << std::endl;
            break;
        } else if (!FD_ISSET(sock, &readSet)) {
            continue;
        }

        std::cout << "Receiving message: " << buff << std::endl;

        // parse packets


        // execute request

    }



    closeSocket(sock);
    cleanup();

    return 0;
}

