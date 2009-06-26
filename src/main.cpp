#include "global.h"
#include "dbsetup.h"
#include "option.h"
#include "optionssetup.h"
#include "irc/ircserver.h"
#include "freenet/freenetconnection.h"

int main()
{
	int loglevel=LogFile::LOGLEVEL_DEBUG;
	Option option;
	IRCServer irc;
	FreenetConnection fn;

	global::db.Open("flip.db3");
	SetupDB(&global::db);
	SetupDefaultOptions(&global::db);

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
