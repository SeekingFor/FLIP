#ifndef _dbsetup_
#define _dbsetup_

#include <string>

#include "db/sqlite3db.h"

// opens database and creates tables and initial inserts if necessary
void SetupDB(SQLite3DB::DB *db);
// verifies DB isn't corrupt
const bool VerifyDB(SQLite3DB::DB *db);

// returns result of PRAGMA integrity_check
const std::string TestDBIntegrity(SQLite3DB::DB *db);

#endif	// _dbsetup_
