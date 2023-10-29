#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>

#include "SessionsDAO.h"

SessionsDAO::SessionsDAO(std::shared_ptr<DatabaseManager> databaseManager_)
	: databaseManager(databaseManager_) {

}

bool SessionsDAO::sessionExists(uint32_t sessionID) const {
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

int SessionsDAO::createSession(std::string username_) const {
	try {
		// get the user ID from the username
		uint32_t sessionID = 0;
		std::vector<std::vector<mysqlx::Value>> userResult = databaseManager->query("users")->select({"id"})->where("username = \"" + username_ + "\"")->execute();
		if (userResult.size() != 1) {
			return -1;
		} else {
			sessionID = userResult[0][0].get<uint32_t>();
		}

		// get current time
		time_t now = time(0);
		struct tm tstruct;
		char buf[80];
		localtime_s(&tstruct , &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

		// create the new session
		std::vector<std::vector<mysqlx::Value>> insertResult = databaseManager->query("sessions")->insert({std::to_string(sessionID), std::string(buf)}, {"user_id", "creation_datetime"})->execute();
		return 0;//TODO: get new session ID from database
	} catch (std::exception e) {
		std::cout << "Create Session Exception: " << e.what() << std::endl;
	}
	return -1;
}