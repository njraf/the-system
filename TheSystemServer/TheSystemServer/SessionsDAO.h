#pragma once

#include "DatabaseManager.h"

class SessionsDAO {

public:
	SessionsDAO(std::shared_ptr<DatabaseManager> databaseManager_);
	~SessionsDAO() = default;

	bool sessionExists(uint32_t sessionID) const;
	uint32_t createSession(std::string username_) const;

private:
	std::shared_ptr<DatabaseManager> databaseManager;
};

