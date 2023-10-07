#include "DatabaseManager.h"

#include <iostream>

DatabaseManager::DatabaseManager()
    : schema("")
{
        
}
DatabaseManager::DatabaseManager(std::shared_ptr<mysqlx::Session> session_, std::string initialSchema)
    : schema(initialSchema)
    , session(session_)
{

}

DatabaseManager::~DatabaseManager() {
    
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


