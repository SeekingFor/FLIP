#include "optionssetup.h"
#include "db/sqlite3db.h"
#include "global.h"

#include <string>
#include <sstream>

void SetupDefaultOptions(SQLite3DB::DB *db)
{
	// OptionValue should always be inserted as a string, even if the option really isn't a string - just to keep the field data type consistent

	db->Execute("BEGIN;");

	std::ostringstream tempstr;	// must set tempstr to "" between db inserts
	SQLite3DB::Statement st=db->Prepare("INSERT INTO tblOption(Option,OptionValue) VALUES(?,?);");
	SQLite3DB::Statement upd=db->Prepare("UPDATE tblOption SET Section=?, SortOrder=?, ValidValues=?, OptionDescription=?, DisplayType=?, DisplayParam1=?, DisplayParam2=?, Mode=? WHERE Option=?;");
	int order=0;

	// LogLevel
	tempstr.str("");
	tempstr << LogFile::LOGLEVEL_DEBUG;
	st.Bind(0,"LogLevel");
	st.Bind(1,tempstr.str());
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"0|0 - Fatal Errors|1|1 - Errors|2|2 - Warnings|3|3 - Informational Messages|4|4 - Debug Messages|5|5 - Trace Messages");
	upd.Bind(3,"The maximum logging level that will be written to file.  Higher levels will include all messages from the previous levels.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"LogLevel");
	upd.Step();
	upd.Reset();

	st.Bind(0,"VacuumOnStartup");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"VACUUM the database every time FLIP starts.  This will defragment the free space in the database and create a smaller database file.  Vacuuming the database can be CPU and disk intensive.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"VacuumOnStartup");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageBase");
	st.Bind(1,"flip");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"A unique string shared by all clients who want to communicate with each other.  This should not be changed unless you want to create your own separate communications network.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MessageBase");
	upd.Step();
	upd.Reset();

	// IRCMOTD
	st.Bind(0,"IRCMOTD");
	st.Bind(1,"");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Message of the day.  Separate lines by a single newline (\\n) character.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCMOTD");
	upd.Step();
	upd.Reset();

	// IRCBindAddresses
	st.Bind(0,"IRCBindAddresses");
	st.Bind(1,"localhost,127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"A comma separated list of valid IPv4 or IPv6 addresses/hostnames that the IRC service will try to bind to.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"IRCBindAddresses");
	upd.Step();
	upd.Reset();

	// IRCListenUnsecure
	st.Bind(0,"IRCListenUnsecure");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Start listening for unsecured connections.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCListenUnsecure");
	upd.Step();
	upd.Reset();

	// IRCListenPort
	st.Bind(0,"IRCListenPort");
	st.Bind(1,"6667");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that the IRC service will listen for unsecured incoming connections.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"IRCListenPort");
	upd.Step();
	upd.Reset();

	// IRCSSLListen
	st.Bind(0,"IRCListenSSL");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Start listening for SSL connections on the SSL port.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCListenSSL");
	upd.Step();
	upd.Reset();

	// IRCSSLListenPort
	st.Bind(0,"IRCSSLListenPort");
	st.Bind(1,"6697");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that the IRC service will listen for SSL incoming connections.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCSSLListenPort");
	upd.Step();
	upd.Reset();

	// IRCSSLCertificate
	st.Bind(0,"IRCSSLCertificate");
	st.Bind(1,"");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"PEM base64 encoded certificate.  You must manually create and export the certificate and place the contents here.");
	upd.Bind(4,"textarea");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCSSLCertificate");
	upd.Step();
	upd.Reset();

	// IRCSSLRSAKey
	st.Bind(0,"IRCSSLRSAKey");
	st.Bind(1,"");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"PEM base64 encoded RSA key.  You must manually create and export the key and place the contents here.");
	upd.Bind(4,"textarea");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCSSLRSAKey");
	upd.Step();
	upd.Reset();

	// IRCSSLRSAPassword
	st.Bind(0,"IRCSSLRSAPassword");
	st.Bind(1,"");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Password to access RSA key.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCSSLRSAPassword");
	upd.Step();
	upd.Reset();

	// IRCSSLDHPrime
	st.Bind(0,"IRCSSLDHPrime");
	st.Bind(1,"E4004C1F94182000103D883A448B3F802CE4B44A83301270002C20D0321CFD0011CCEF784C26A400F43DFB901BCA7538F2C6B176001CF5A0FD16D2C48B1D0C1CF6AC8E1DA6BCC3B4E1F96B0564965300FFA1D0B601EB2800F489AA512C4B248C01F76949A60BB7F00A40B1EAB64BDD48E8A700D60B7F1200FA8E77B0A979DABF");
	st.Step();
	st.Reset();
	upd.Bind(0,"IRC Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"DH-1024 prime in hex format.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"IRCSSLDHPrime");
	upd.Step();
	upd.Reset();

	// FCPHost
	st.Bind(0,"FCPHost");
	st.Bind(1,"127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Host name or address of Freenet node.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FCPHost");
	upd.Step();
	upd.Reset();

	// FCPPort
	st.Bind(0,"FCPPort");
	st.Bind(1,"9481");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that Freenet is listening for FCP connections on.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FCPPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageInsertPriority");
	st.Bind(1,"2");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2,"0|0|1|1|2|2|3|3|4|4|5|5|6|6");
	upd.Bind(3,"The priority class for message inserts.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MessageInsertPriority");
	upd.Step();
	upd.Reset();

	db->Execute("COMMIT;");

}
