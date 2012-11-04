#include "dbsetup.h"
#include "dbconversions.h"
#include "option.h"
#include "db/sqlite3db.h"

void SetupDB(SQLite3DB::DB *db)
{

	std::string tempval="";

	db->Execute("CREATE TABLE IF NOT EXISTS tblDBVersion(\
				Major				INTEGER,\
				Minor				INTEGER\
				);");

	SQLite3DB::Statement st=db->Prepare("SELECT Major,Minor FROM tblDBVersion;");
	st.Step();
	if(st.RowReturned())
	{
		int major;
		int minor;
		st.ResultInt(0,major);
		st.ResultInt(1,minor);
		st.Finalize();
		
		if(major==0 && minor==1)
		{
			ConvertDB0001To0002(db);
			major=0;
			minor=2;
		}
	}
	else
	{
		db->Execute("INSERT INTO tblDBVersion(Major,Minor) VALUES(0,2);");
	}

	db->Execute("UPDATE tblDBVersion SET Major=0, Minor=2;");

	db->Execute("CREATE TABLE IF NOT EXISTS tblOption(\
				Option				TEXT UNIQUE,\
				OptionValue			TEXT NOT NULL,\
				OptionDescription	TEXT,\
				Section				TEXT,\
				SortOrder			INTEGER,\
				ValidValues			TEXT,\
				DisplayType			TEXT CHECK (DisplayType IN ('textbox','textarea','select','multiselect')) DEFAULT 'textbox',\
				DisplayParam1		TEXT,\
				DisplayParam2		TEXT,\
				Mode				TEXT CHECK (Mode IN ('simple','advanced')) DEFAULT 'simple'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblLocalIdentity(\
				LocalIdentityID			INTEGER PRIMARY KEY,\
				Name					TEXT,\
				PublicKey				TEXT UNIQUE,\
				PrivateKey				TEXT UNIQUE,\
				RSAPublicKey			TEXT UNIQUE,\
				RSAPrivateKey			TEXT UNIQUE,\
				DateAdded				DATETIME\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentity(\
				IdentityID				INTEGER PRIMARY KEY,\
				PublicKey				TEXT UNIQUE,\
				RSAPublicKey			TEXT UNIQUE,\
				Name					TEXT,\
				DateAdded				DATETIME,\
				LastSeen				DATETIME,\
				LastMessage				DATETIME,\
				Ignored					BOOL CHECK(Ignored IN(0,1)) DEFAULT 0,\
				AddedMethod				TEXT,\
				FailureCount			INTEGER CHECK(FailureCount>=0) DEFAULT 0\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentityEdition(\
				IdentityID				INTEGER,\
				Date					DATE,\
				Edition					INTEGER,\
				Found					BOOL CHECK(Found IN(0,1)) DEFAULT 0\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_IdentityEdition ON tblIdentityEdition(IdentityID,Date,Edition);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblInsertedMessageIndex(\
				LocalIdentityID			INTEGER,\
				Date					DATE,\
				MessageIndex			INTEGER,\
				Inserted				BOOL CHECK(Inserted IN (0,1)) DEFAULT 0\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_InsertedMessageIndex ON tblInsertedMessageIndex(LocalIdentityID,Date,MessageIndex);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblRetrievedMessageIndex(\
				IdentityID				INTEGER,\
				Date					DATE,\
				MessageIndex			INTEGER,\
				Found					BOOL CHECK(Found IN (0,1)) DEFAULT 0,\
				Tries					INTEGER CHECK(Tries>=0) DEFAULT 0\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_RetrievedMessageIndex ON tblRetrievedMessageIndex(IdentityID,Date,MessageIndex);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblAnnounceIndex(\
				Date					DATE,\
				AnnounceIndex			INTEGER,\
				Done					BOOL CHECK(Done IN (0,1)) DEFAULT 0\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_AnnounceIndex ON tblAnnounceIndex(Date,AnnounceIndex);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblChannel(\
		ChannelID			INTEGER PRIMARY KEY,\
		Name				TEXT,\
		Topic				TEXT\
		);");

	// run analyze - may speed up some queries
	db->Execute("ANALYZE;");

}

const bool VerifyDB(SQLite3DB::DB *db)
{
	SQLite3DB::Statement st=db->Prepare("PRAGMA integrity_check;");
	st.Step();
	if(st.RowReturned())
	{
		std::string res="";
		st.ResultText(0,res);
		if(res=="ok")
		{
			return true;
		}
		else
		{
			// try to reindex and vacuum database in case of index corruption
			st=db->Prepare("REINDEX;");
			st.Step();
			st=db->Prepare("VACUUM;");
			st.Step();

			// check integrity again
			st=db->Prepare("PRAGMA integrity_check;");
			st.Step();
			st.ResultText(0,res);
			if(res=="ok")
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

const std::string TestDBIntegrity(SQLite3DB::DB *db)
{
	std::string result="";

	SQLite3DB::Statement st=db->Prepare("PRAGMA integrity_check;");
	st.Step();
	while(st.RowReturned())
	{
		std::string text="";
		st.ResultText(0,text);
		result+=text;
		st.Step();
	}
	return result;
}
