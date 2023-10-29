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
		DatabaseManager::IntermediateQuery *select(const std::vector<std::string> &columns_ = {});
		DatabaseManager::IntermediateQuery *insert(const std::vector<std::string> &values_, const std::vector<std::string> &columns_ = {});
		DatabaseManager::IntermediateQuery *where(std::string condition_);
	private:
		DatabaseManager *dbm;
		std::vector<std::string> query;

		std::map<std::string, std::vector<std::string>> executeAsMap(); //TODO: move back to public when complete
	};

public:
	DatabaseManager();
	DatabaseManager(std::shared_ptr<mysqlx::Session> session_, std::string initialSchema);
	~DatabaseManager();

	mysqlx::Schema getSchema();

	void printTable(mysqlx::Table table);
	std::shared_ptr<DatabaseManager::IntermediateQuery> query(std::string table_);

private:
	std::string schema;
	std::shared_ptr<mysqlx::Session> session;
};

