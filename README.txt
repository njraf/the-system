Description:
This is a system design project that will have a server and client. Servers will be able to scale horizontally to handle requests from a large number of clients. A load balancer will be used to direct client requests to reduce strain on any given server. Servers will have access to a database for persistent data storage.

Technologies:
Most of the project will be written in c++ where the server and load balancer will be console applications and the client will be a Qt project. Additional client may be created in the future such as an android app written in kotlin. Servers will access a MySQL database and if clients end up needing a database to store data locally then SQLite will be used.

Crossplatform compatibility:
The server and load balancer will use preprocessor directives to allow the use of both Windows and Berkley sockets so that they can run on any platform. Qt is able to compile on both Windows and Linux, which allows the client application to run on either platform. A kotlin multiplatform project may also be created to bring a client app to both android and iOS with one codebase written in kotlin.

Dependencies:
[Server]
- MySQL connector/c++ 8.1.0
	- https://dev.mysql.com/downloads/connector/cpp/
	- Windows file path C:\Program Files\MySQL\mysql-connector-c++-8.1.0-winx64
	- Linux compile options: -I /path/to/connector/include -L /path/to/connector/lib64 -lmysqlcppconn8
	- add mysqlcppconn8-2-vs14.dll alongside source files (or the executable if the project is deployed) to use the api
	- create a file alongside source files (or the executable if the project is deployed) called credentials.txt with the username on the first line and password on the second line
	
[Load Balancer]
- none

[Qt client]
- none
