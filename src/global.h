#ifndef _global_
#define _global_

#include "logfile.h"
#include "db/sqlite3db.h"

#define FLIP_MAJOR		"0"
#define FLIP_MINOR		"1"
#define FLIP_RELEASE	"1"
#define FLIP_VERSION	FLIP_MAJOR"."FLIP_MINOR"."FLIP_RELEASE

namespace global
{

extern LogFile log;
extern SQLite3DB::DB db;

}

#endif	// _global_
