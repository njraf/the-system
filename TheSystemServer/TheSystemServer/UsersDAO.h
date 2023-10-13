#pragma once

#include <string>

#include "DatabaseManager.h"

class UsersDAO {

public:
	UsersDAO(std::shared_ptr<DatabaseManager> databaseManager_);
	~UsersDAO() = default;

	bool userExists(const std::string &username) const;
	bool isValidSignInAttempt(const std::string &username, const std::string &password) const;


private:
	std::shared_ptr<DatabaseManager> databaseManager;

};

