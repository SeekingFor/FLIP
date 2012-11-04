#include "dbconversions.h"
#include "db/sqlite3db.h"
#include "stringfunctions.h"

void ConvertDB0001To0002(SQLite3DB::DB *db)
{
	db->Execute("CREATE TABLE IF NOT EXISTS tblChannel(\
		ChannelID			INTEGER PRIMARY KEY,\
		Name				TEXT,\
		Topic				TEXT\
		);");

	db->Execute("ALTER TABLE tblAnnounceIndex ADD COLUMN Done BOOL CHECK(Done IN (0,1)) DEFAULT 0;");
}
