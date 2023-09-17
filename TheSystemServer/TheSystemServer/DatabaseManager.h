#pragma once

#include <mysqlx/xdevapi.h>

class DatabaseManager {

public:
	DatabaseManager(std::string address, int port, std::string username, std::string password, std::string initialSchema);
	~DatabaseManager();

	mysqlx::Schema getSchema();

	void printTable(mysqlx::Table table);

	void test() {
		mysqlx::Session sess("localhost", 33060, "user", "password");
		mysqlx::Schema db = sess.getSchema("the_system");
		// or Schema db(sess, "test");

		mysqlx::Collection myColl = db.getCollection("my_collection");
		// or Collection myColl(db, "my_collection");

		mysqlx::DocResult myDocs = myColl.find("name like :param")
			.limit(1)
			.bind("param", "L%").execute();

		//cout << myDocs.fetchOne();


	}


private:
	mysqlx::Session session;
	mysqlx::Schema schema;
};

