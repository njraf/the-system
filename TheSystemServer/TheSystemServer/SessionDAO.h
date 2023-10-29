#pragma once

#include "DatabaseManager.h"

class SessionDAO {

public:
	SessionDAO(std::shared_ptr<DatabaseManager> databaseManager_);
	~SessionDAO() = default;

	bool sessionExists(uint32_t sessionID) const;


private:
	std::shared_ptr<DatabaseManager> databaseManager;
};

