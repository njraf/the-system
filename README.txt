Description:
This is a system design project that will have a server and client. Servers will be able to scale horizontally to handle requests from a large number of clients. A load balancer will be used to direct client requests to reduce strain on any given server. Servers will have access to a database for persistent data storage.

Technologies:
Most of the project will be written in c++ where the server and load balancer will be console applications and the client will be a Qt project. Additional clients may be created in the future such as an android app written in kotlin. Servers will access a MySQL database and if clients end up needing a database to store data locally then SQLite will be used.

Cross platform compatibility:
The server and load balancer will use preprocessor directives to allow the use of both Windows and Berkley sockets so that they can run on any platform. Qt is able to compile on both Windows and Linux, which allows the client application to run on either platform. A kotlin multiplatform project may also be created to bring a client app to both android and iOS with one codebase written in kotlin.

Dependencies:
[All]
- create an addresses.csv at the root of the repository with the following format:
	- load_balancer,<ip address>
	- server,<ip address>
	- desktop_client,<ip address>

[MySQL]
- Use the provided sql dump to construct the database and any tables that it has
	- This can be done in MySQL Workbench by importing the sql dump
	- WARNING: make sure no other databases have the name "the_system" so they are not overwritten

[Server]
- Requires zlib to be dynamically linked for crc32 checksum
- Create a MySQL database using the provided sql dump as mentioned in the [MySQL] dependency
- MySQL connector/c++ 8.1.0
	- https://dev.mysql.com/downloads/connector/cpp/
	- Windows file path C:\Program Files\MySQL\mysql-connector-c++-8.1.0-winx64
	- Linux filepath "/usr/local/lib/mysql-connector-c++-8.1.0-linux-glibc2.28-x86_64bit"
	- Linux compile options: -I </path/to/connector>/include -L </path/to/connector?/lib64 -lmysqlcppconn8
	- On Windows: add mysqlcppconn8-2-vs14.dll alongside source files (or the executable if the project is deployed) to use the api
	- On Linux:
		- type "locate libmysqlcppconn8" to find the path to the shared library
		- optionally, create a symlink to the shared library
		- add the filepath that contains the shared library (or the symlink to it) to the LD_LIBRARY_PATH environment variable in .bashrc
- create a credentials.txt with the database username and password on lines one and two respectively
	- place this in the same folder as the server source files, or the executable if the project is deployed
- the mysql api uses c++17 feature and requires the server to be built using c++17 as a minimum requirement
	
[Load Balancer]
- uses c++11 features

[Qt client]
- Requires zlib to be dynamically linked for crc32 checksum
- uses c++11 features

