#ifndef _idatabase_
#define _idatabase_

#include "global.h"
#include "db/sqlite3db.h"

/**
	\brief Base class for classes that need to access the Singleton SQLite 3 database object
*/
class IDatabase
{
public:
	IDatabase():m_db(&global::db) {}
	
protected:
	SQLite3DB::DB *m_db;
};

#endif	// _idatabase_
