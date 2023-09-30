#include "DatabaseManager.h"

#include <iostream>

DatabaseManager::DatabaseManager(std::string address, int port, std::string username, std::string password, std::string initialSchema)
	: session(address, port, username, password)
	, schema(session.getSchema(initialSchema)) {

    session.sql(mysqlx::string("USE ") + mysqlx::string(initialSchema) + mysqlx::string(";")).execute();
}

DatabaseManager::~DatabaseManager() {
    session.close();
}

mysqlx::Schema DatabaseManager::getSchema() {
	return schema;
}

void DatabaseManager::printTable(mysqlx::Table table) {

    // print column names
    for (int r = 0; r < 1; r++) {
        mysqlx::RowResult row = table.select().execute();
        for (int c = 0; c < row.getColumnCount(); c++) {
            std::cout << row.getColumn(c).getColumnName() << "\t";
        }
        std::cout << std::endl;
    }

    mysqlx::RowResult rows = table.select().execute();
    mysqlx::Row row;
    while (row = rows.fetchOne()) {
        for (int col = 0; col < row.colCount(); col++) {
            std::cout << row[col] << "\t";
        }
        std::cout << std::endl;
    }
}


