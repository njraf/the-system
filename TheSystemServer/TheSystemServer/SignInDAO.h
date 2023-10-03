#pragma once

#include <string>

#include "DatabaseManager.h"

class SignInDAO {

public:
	SignInDAO(std::shared_ptr<DatabaseManager> databaseManager_);
	~SignInDAO() = default;

	bool userExists(std::string username);


private:
	std::shared_ptr<DatabaseManager> databaseManager;

};

