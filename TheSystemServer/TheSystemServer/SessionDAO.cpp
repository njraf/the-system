#include <iostream>
#include <string>

#include "SessionDAO.h"

SessionDAO::SessionDAO(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool SessionDAO::sessionExists(uint32_t sessionID) const {
	try {
		std::vector<std::vector<mysqlx::Value>> result = databaseManager->query("sessions")->select({"COUNT(id)"})->where("id = " + std::to_string(sessionID))->execute();
		if (result.size() == 1) {
			return true;
		}
	} catch (std::exception e) {
		std::cout << "Session Exists Exception: " << e.what() << std::endl;
	}
	return false;
}