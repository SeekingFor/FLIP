#ifndef _dbconversions_
#define _dbconversions_

#include "db/sqlite3db.h"

void ConvertDB0001To0002(SQLite3DB::DB *db);
void ConvertDB0002To0003(SQLite3DB::DB *db);
void ConvertDB0003To0004(SQLite3DB::DB *db);

#endif	// _dbconversions_
