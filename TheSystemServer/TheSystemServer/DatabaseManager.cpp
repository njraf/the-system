#include "DatabaseManager.h"

#include <iostream>
#include <algorithm>

DatabaseManager::DatabaseManager()
    : schema("")
{
        
}
DatabaseManager::DatabaseManager(std::shared_ptr<mysqlx::Session> session_, std::string initialSchema)
    : schema(initialSchema)
    , session(session_)
{
    session->sql("USE " + schema + ";").execute();
}

DatabaseManager::~DatabaseManager() {
    
}

mysqlx::Schema DatabaseManager::getSchema() {
	return session->getSchema(schema);
}

void DatabaseManager::printTable(mysqlx::Table table) {

    // print column names
    mysqlx::RowResult rows = table.select().execute();
    for (int c = 0; c < rows.getColumnCount(); c++) {
        std::cout << rows.getColumn(c).getColumnName() << "\t";
    }
    std::cout << std::endl;

    // print rows
    mysqlx::Row row;
    while (row = rows.fetchOne()) {
        for (int col = 0; col < row.colCount(); col++) {
            std::cout << row[col] << "\t";
        }
        std::cout << std::endl;
    }
}

std::shared_ptr<DatabaseManager::IntermediateQuery> DatabaseManager::query(std::string table_) {
    return std::make_shared<DatabaseManager::IntermediateQuery>(this, table_);
}


// IntermediateQuery //

DatabaseManager::IntermediateQuery::IntermediateQuery(DatabaseManager *dbm_, std::string table_)
    : dbm(dbm_)
{
    query.push_back(table_);
}

std::vector<std::vector<mysqlx::Value>> DatabaseManager::IntermediateQuery::execute() {
    std::string queryString = "";
    for (std::string s : query) {
        queryString += (s + " ");
    }
    queryString += ";";

    //std::cout << "Query: " << queryString << std::endl;
    mysqlx::SqlResult result = dbm->session->sql(queryString).execute();

    // construct results
    std::vector<mysqlx::Row> rows = result.fetchAll();
    std::vector<std::vector<mysqlx::Value>> output(rows.size());

    std::transform(rows.begin(), rows.end(), output.begin(), [](mysqlx::Row row) {
        std::vector<mysqlx::Value> outRow;
        for (int r = 0; r < row.colCount(); r++) {
            outRow.push_back(row.get(r));
        }
        return outRow;
    });
    return output;
}

//TODO
std::map<std::string, std::vector<std::string>> DatabaseManager::IntermediateQuery::executeAsMap() {
    return std::map<std::string, std::vector<std::string>>();
}

DatabaseManager::IntermediateQuery* DatabaseManager::IntermediateQuery::select(const std::vector<std::string> &columns_) {
    query.insert(query.begin(), "FROM");
    for (size_t i = 0; i < columns_.size(); i++) {
        std::string value = columns_[i];
        if ((i + 1) != columns_.size()) {
            value += ",";
        }
        query.insert(query.begin() + i, value);
    }

    if (columns_.empty()) {
        query.insert(query.begin(), "*");
    }
    query.insert(query.begin(), "SELECT");
    return this;
}

DatabaseManager::IntermediateQuery *DatabaseManager::IntermediateQuery::insert(const std::vector<std::string> &values_, const std::vector<std::string> &columns_) {
    query.insert(query.begin(), {"INSERT", "INTO"});
    if (!columns_.empty()) {
        query.push_back("(");
        for (size_t i = 0; i < columns_.size(); i++) {
            std::string value = columns_[i];
            if ((i + 1) != columns_.size()) {
                value += ",";
            }
            query.push_back(value);
        }
        query.push_back(")");
        query.push_back("VALUES");
        query.push_back("(");
        for (size_t i = 0; i < values_.size(); i++) {
            std::string value = "\"" + values_[i] + "\"";
            if ((i + 1) != values_.size()) {
                value += ",";
            }
            query.push_back(value);
        }
        query.push_back(")");
    }
    return this;
}
