#include "UsersDAO.h"

#include <iostream>

UsersDAO::UsersDAO(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool UsersDAO::userExists(const std::string &username) const {
	try {
		std::vector<std::vector<mysqlx::Value>> result = databaseManager->query("users")->select()->execute();
		for (auto row : result) {
			if (username == row[3].get<mysqlx::string>()) {
				return true;
			}
		}
	} catch (std::exception e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	return false;
}

bool UsersDAO::isValidSignInAttempt(const std::string &username, const std::string &password) const {
	try {
		std::vector<std::vector<mysqlx::Value>> result = databaseManager->query("users")->select()->execute();
		for (auto row : result) {
			std::cout << "Comparing " << username << " = " << row[3].get<mysqlx::string>() << " | " << password << " = " << row[4].get<mysqlx::string>() << std::endl;
			if ((username == row[3].get<mysqlx::string>()) && (password == row[4].get<mysqlx::string>())) {
				return true;
			}
		}
	} catch (std::exception e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	return false;
}

