#pragma once

#include <vector>
#include <map>
#include <memory>

#include <mysqlx/xdevapi.h>

class DatabaseManager {

private:
	class IntermediateQuery {
	public:
		IntermediateQuery(DatabaseManager *dbm_, std::string table_);
		~IntermediateQuery() = default;
		std::vector<std::vector<mysqlx::Value>> execute();
		std::map<std::string, std::vector<std::string>> executeAsMap();
		DatabaseManager::IntermediateQuery* select(std::vector<std::string> columns_ = {});
	private:
		DatabaseManager *dbm;
		std::string table = "";
		std::vector<std::string> columns;
		std::string where = "";
	};

public:
	DatabaseManager();
	DatabaseManager(std::shared_ptr<mysqlx::Session> session_, std::string initialSchema);
	~DatabaseManager();

	mysqlx::Schema getSchema();

	void printTable(mysqlx::Table table);
	std::shared_ptr<DatabaseManager::IntermediateQuery> query(std::string table_);


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
	std::string schema;
	std::shared_ptr<mysqlx::Session> session;
};

