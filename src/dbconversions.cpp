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

void ConvertDB0002To0003(SQLite3DB::DB *db)
{
	db->Execute("CREATE TABLE tmpIdentity AS SELECT * FROM tblIdentity;");
	db->Execute("DROP TABLE tblIdentity;");
	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentity(\
				IdentityID				INTEGER PRIMARY KEY,\
				PublicKey				TEXT,\
				RSAPublicKey			TEXT,\
				Name					TEXT,\
				DateAdded				DATETIME,\
				LastSeen				DATETIME,\
				LastMessage				DATETIME,\
				Ignored					BOOL CHECK(Ignored IN(0,1)) DEFAULT 0,\
				AddedMethod				TEXT,\
				FailureCount			INTEGER CHECK(FailureCount>=0) DEFAULT 0\
				);");
	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxIdentity_PublicKey ON tblIdentity(PublicKey,RSAPublicKey);");
	db->Execute("INSERT INTO tblIdentity(IdentityID,PublicKey,RSAPublicKey,Name,DateAdded,LastSeen,LastMessage,Ignored,AddedMethod,FailureCount) SELECT IdentityID,PublicKey,RSAPublicKey,Name,DateAdded,LastSeen,LastMessage,Ignored,AddedMethod,FailureCount FROM tmpIdentity;");
	db->Execute("DROP TABLE tmpIdentity;");
}
