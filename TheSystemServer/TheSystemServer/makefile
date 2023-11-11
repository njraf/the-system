#!/bin/bash

CC=g++
MYSQL_CPPCON_DIR = /usr/local/lib/mysql-connector-c++-8.1.0-linux-glibc2.28-x86-64bit
CPPFLAGS =-I $(MYSQL_CPPCON_DIR)/include -L $(MYSQL_CPPCON_DIR)/lib64
LDLIBS = -lmysqlcppconn8 
CXXFLAGS = -std=c++17


all:
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o server TheSystemServer.cpp sockets.cpp DatabaseManager.cpp RequestHandler.cpp UsersDAO.cpp SessionsDAO.cpp PacketFormat.cpp $(LDLIBS)
clean:
	@rm server
