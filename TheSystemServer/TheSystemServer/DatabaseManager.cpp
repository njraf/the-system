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
    , table(table_)
{
}

std::vector<std::vector<mysqlx::Value>> DatabaseManager::IntermediateQuery::execute() {
    // construct query
    std::string query = "SELECT";
    if (columns.empty()) {
        //TODO: select all columns. this implies that select() is not necessary
        query += " *";
    } else {
        for (std::string c : columns) {
            query += " " + c;
        }
    }

    query += " FROM " + table;


    query += ";";
    mysqlx::SqlResult result = dbm->session->sql(query).execute();

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

DatabaseManager::IntermediateQuery* DatabaseManager::IntermediateQuery::select(std::vector<std::string> columns_) {
    columns = columns_;
    return this;
}
