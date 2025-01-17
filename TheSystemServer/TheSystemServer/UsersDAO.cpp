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
		std::cout << "User Exists Exception: " << e.what() << std::endl;
	}
	return false;
}

bool UsersDAO::isValidSignInAttempt(const std::string &username, const std::string &password) const {
	try {
		std::vector<std::vector<mysqlx::Value>> result = databaseManager->query("users")->select({"username", "password"})->execute();
		for (auto row : result) {
			if ((username == row[0].get<mysqlx::string>()) && (password == row[1].get<mysqlx::string>())) {
				return true;
			}
		}
	} catch (std::exception e) {
		std::cout << "Signin Attempt Exception: " << e.what() << std::endl;
	}
	return false;
}

bool UsersDAO::createUser(const std::string &firstName, const std::string &lastName, const std::string &username, const std::string &password) {
	try {
		databaseManager->query("users")->insert({firstName, lastName, username, password}, {"first_name", "last_name", "username", "password"})->execute();
		return true;
	} catch (std::exception e) {
		std::cout << "Create User Exception: " << e.what() << std::endl;
	}
	return false;
}
