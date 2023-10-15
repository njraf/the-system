#pragma once

#include <string>

#include "DatabaseManager.h"

class UsersDAO {

public:
	UsersDAO(std::shared_ptr<DatabaseManager> databaseManager_);
	~UsersDAO() = default;

	bool userExists(const std::string &username) const;
	bool isValidSignInAttempt(const std::string &username, const std::string &password) const;
	bool createUser(const std::string &firstName, const std::string &lastName, const std::string &username, const std::string &password);

private:
	std::shared_ptr<DatabaseManager> databaseManager;

};

