#include "global.h"

namespace global
{

LogFile log;
SQLite3DB::DB db;
volatile bool shutdown;
bool daemon;

}
