#pragma once

#include <vector>
#include <map>
#include <memory>

#include <mysqlx/xdevapi.h>

enum class Keyword : uint32_t {
	NOW = 0
};

class DatabaseManager {

private:
	class IntermediateQuery {
	public:
		IntermediateQuery(DatabaseManager *dbm_, std::string table_);
		~IntermediateQuery() = default;
		std::vector<std::vector<mysqlx::Value>> execute();

		DatabaseManager::IntermediateQuery *select(const std::vector<std::string> &columns_ = {});
		DatabaseManager::IntermediateQuery *insert(const std::vector<std::string> &values_, const std::vector<std::string> &columns_ = {});
		DatabaseManager::IntermediateQuery *where(std::string condition_);
		DatabaseManager::IntermediateQuery *remove();
	private:
		DatabaseManager *dbm;
		std::vector<std::string> query;

		std::map<std::string, std::vector<std::string>> executeAsMap(); //TODO: move back to public when complete
	};

public:
	DatabaseManager();
	DatabaseManager(std::shared_ptr<mysqlx::Session> session_, std::string initialSchema);
	~DatabaseManager();

	std::shared_ptr<mysqlx::Session> getSession();
	mysqlx::Schema getSchema();

	void printTable(mysqlx::Table table);
	std::shared_ptr<DatabaseManager::IntermediateQuery> query(std::string table_);

	static std::map<Keyword, std::string> keywordToString;

private:
	std::string schema;
	std::shared_ptr<mysqlx::Session> session;
};

