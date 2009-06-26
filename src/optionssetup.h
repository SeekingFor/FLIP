#ifndef _optionssetup_
#define _optionssetup_

#include "db/sqlite3db.h"

// inserts default options into the database
void SetupDefaultOptions(SQLite3DB::DB *db);

#endif	// _optionssetup_
