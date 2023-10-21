#!/bin/bash

all:
	MYSQL_CPPCONN_DIR=/usr/local/lib/mysqlcppconn/usr
	@g++ -I/usr/local/lib/mysqlcppconn/usr/include/mysql-cppconn-8 -L/usr/local/lib/mysqlcppconn/usr/lib/x86_64-linux-gnu -lmysqlcppconn8 -o server TheSystemServer.cpp sockets.cpp DatabaseManager.cpp RequestHandler.cpp UsersDAO.cpp PacketFormat.cpp
clean:
	@rm server
