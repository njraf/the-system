#include "SignInDAO.h"

SignInDAO::SignInDAO(std::shared_ptr<DatabaseManager> databaseManager_) 
	: databaseManager(databaseManager_)
{

}

bool SignInDAO::userExists(std::string username) {


	return true;
}

