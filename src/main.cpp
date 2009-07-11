#include "global.h"
#include "dbsetup.h"
#include "option.h"
#include "optionssetup.h"
#include "irc/ircserver.h"
#include "freenet/freenetconnection.h"
#include "rsakeypair.h"

#include <cstdio>

void creatersakeypairs(SQLite3DB::DB *db)
{
	SQLite3DB::Statement updatest=db->Prepare("UPDATE tblLocalIdentity SET RSAPublicKey=?, RSAPrivateKey=? WHERE LocalIdentityID=?;");
	SQLite3DB::Statement st=db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE RSAPrivateKey IS NULL OR RSAPrivateKey='';");
	st.Step();
	while(st.RowReturned())
	{
		int localidentityid=0;
		RSAKeyPair rsa;
		rsa.Generate();

		st.ResultInt(0,localidentityid);

		updatest.Bind(0,rsa.GetEncodedPublicKey());
		updatest.Bind(1,rsa.GetEncodedPrivateKey());
		updatest.Bind(2,localidentityid);
		updatest.Step();
		updatest.Reset();

		st.Step();
	}
}

int main()
{

	int loglevel=LogFile::LOGLEVEL_DEBUG;
	Option option;
	global::db.Open("flip.db3");

	SetupDB(&global::db);
	SetupDefaultOptions(&global::db);

	creatersakeypairs(&global::db);

	IRCServer irc;
	FreenetConnection fn;

	unlink("flip-5.log");
	rename("flip-4.log","flip-5.log");
	rename("flip-3.log","flip-4.log");
	rename("flip-2.log","flip-3.log");
	rename("flip-1.log","flip-2.log");
	rename("flip.log","flip-1.log");

	global::log.SetFileName("flip.log");
	global::log.OpenFile();
	global::log.SetWriteDate(true);
	global::log.SetWriteLogLevel(true);
	global::log.SetWriteNewLine(true);
	option.GetInt("LogLevel",loglevel);
	global::log.SetLogLevel((LogFile::LogLevel)loglevel);

	global::log.Info("FLIP startup v"FLIP_VERSION);

	irc.Start();
	do
	{
		irc.Update(50);
		fn.Update(50);
	}while(true);

	irc.Shutdown();

}
