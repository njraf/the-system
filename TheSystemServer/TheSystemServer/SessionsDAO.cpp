#define __STDC_WANT_LIB_EXT1__ 1

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

uint32_t SessionsDAO::createSession(std::string username_) const {
	uint32_t sessionID = 0;
	try {
		// get the user ID from the username
		uint32_t userID = 0;
		std::vector<std::vector<mysqlx::Value>> userResult = databaseManager->query("users")->select({"id"})->where("username = \"" + username_ + "\"")->execute();
		if (userResult.size() != 1) {
			std::cout << "Error: " << userResult.size() << " results" << std::endl;
			return 0;
		} else {
			userID = userResult[0][0].get<uint32_t>();
		}

		// delete all sessions for the users
		databaseManager->query("sessions")->remove()->where("user_id = " + std::to_string(userID))->execute();

		// get current time
		time_t now = time(0);
		struct tm *tstruct;
#ifdef _WIN32
		localtime_s(tstruct, &now);
#else
#ifdef __STDC_LIB_EXT1__
		localtime_s(&now, tstruct);
#else
		tstruct = localtime(&now);
#endif
#endif
		char timeBuff[80];
		strftime(timeBuff, sizeof(timeBuff), "%Y-%m-%d %X", tstruct);

		// create the new session
		databaseManager->query("sessions")->insert({std::to_string(userID), std::string(timeBuff)}, {"user_id", "creation_datetime"})->execute();

		// get the session ID from the user ID
		std::vector<std::vector<mysqlx::Value>> sessionResult = databaseManager->query("sessions")->select({"session_id"})->where("user_id = " + std::to_string(userID))->execute();
		if (sessionResult.size() != 1) {
			std::cout << "Error: " << sessionResult.size() << " results" << std::endl;
			return 0;
		} else {
			sessionID = sessionResult[0][0].get<uint32_t>();
		}
	} catch (std::exception e) {
		std::cout << "Create Session Exception: " << e.what() << std::endl;
		return 0;
	}

	return sessionID;
}
