#include "DatabaseManager.h"

#include <iostream>

DatabaseManager::DatabaseManager(std::string address_, int port_, std::string username_, std::string password_, std::string schema_)
	: address(address)
    , port(port_)
    , username(username_)
    , password(password_)
    , schema(schema_)
    , session(std::make_shared<mysqlx::Session>(address, port, username, password, schema))
{
    session->sql(mysqlx::string("USE ") + mysqlx::string(schema) + mysqlx::string(";")).execute();
}

DatabaseManager::~DatabaseManager() {
    session->close();
}

mysqlx::Schema DatabaseManager::getSchema() {
	return session->getSchema(schema);
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


