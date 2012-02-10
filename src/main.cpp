#include "global.h"
#include "dbsetup.h"
#include "option.h"
#include "optionssetup.h"
#include "irc/ircserver.h"
#include "freenet/freenetconnection.h"
#include "rsakeypair.h"
#include "flipdaemon.h"

#include <cstdio>
#include <vector>
#include <csignal>

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

void showhelp()
{
	SQLite3DB::Statement st=global::db.Prepare("SELECT Option FROM tblOption ORDER BY SortOrder;");
	st.Step();
	std::cout << "FLIP v" << FLIP_VERSION << std::endl;
	std::cout << "parameters:" << std::endl;
	std::cout << "--help                  This help message" << std::endl;
	std::cout << "--daemon                Start FLIP in the background" << std::endl;
	std::cout << "--option=value          Set an option in the database" << std::endl;
	std::cout << "  available options" << std::endl;
	while(st.RowReturned())
	{
		std::string option("");
		st.ResultText(0,option);
		std::cout << "  " << option << std::endl;
		st.Step();
	}
}

void handleoptions(const std::vector<std::string> &options, bool &startup)
{
	Option option;
	std::string val("");

	for(std::vector<std::string>::const_iterator i=options.begin(); i!=options.end(); i++)
	{
		std::string::size_type pos=(*i).find("=");
		if((*i).size()>3 && (*i).substr(0,2)=="--" && pos!=std::string::npos)
		{
			if(option.Get((*i).substr(2,pos-2),val)==true)
			{
				option.Set((*i).substr(2,pos-2),(*i).substr(pos+2));
				startup=false;
			}
		}
		else
		{
			if((*i)=="--help" || (*i)=="-?" || (*i)=="/?")
			{
				showhelp();
				startup=false;
			}
			if((*i)=="--daemon")
			{
				Daemonize();
				global::daemon=true;
			}
		}
	}
}

void handlesignal(int sig)
{
	if(sig==SIGTERM || sig==SIGINT)
	{
		global::shutdown=true;
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT,handlesignal);
	signal(SIGTERM,handlesignal);

	int loglevel=LogFile::LOGLEVEL_DEBUG;
	Option option;
	global::db.Open("flip.db3");
	global::daemon=false;
	global::shutdown=false;
	bool startup=true;

	SetupDB(&global::db);
	SetupDefaultOptions(&global::db);

	creatersakeypairs(&global::db);

	if(argc>1)
	{
		std::vector<std::string> options;

		for(int i=1; i<argc; i++)
		{
			if(argv[i])
			{
				options.push_back(std::string(argv[i]));
			}
		}

		handleoptions(options,startup);
	}

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

	if(startup==true)
	{
		global::log.Info("FLIP startup v"FLIP_VERSION);

		irc.Start();
		do
		{
			irc.Update(50);
			fn.Update(50);
		}while(global::shutdown==false);

		irc.Shutdown();
	}

	if(global::daemon)
	{
		Undaemonize();
	}

	global::log.Info("FLIP shudown complete");

	return 0;
}
