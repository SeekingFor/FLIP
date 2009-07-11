#ifndef _ircserver_
#define _ircserver_

#include "../ilogger.h"
#include "../idatabase.h"
#include "../flipeventsource.h"
#include "../flipeventhandler.h"
#include "../datetime.h"
#include "ircclientconnection.h"
#include "irccommandhandler.h"

#include <vector>
#include <set>
#include <map>

class IRCServer:public IRCCommandHandler,public FLIPEventSource,public FLIPEventHandler,public ILogger,public IDatabase
{
public:
	IRCServer();
	~IRCServer();

	void Start();
	void Shutdown();
	void Update(const unsigned long ms);

	const bool HandleCommand(const IRCCommand &command, IRCClientConnection *client);

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:
	void SendChannelMessageToClients(const int identityid, const std::string &channel, const std::string &message);
	void SendPrivateMessageToClients(const int identityid, const std::string &recipient, const std::string &encryptedmessage);
	void SendPartMessageToClients(const int identityid, const std::string &channel);

	struct idinfo
	{
		std::string m_nick;
		std::string m_publickey;
	};

	DateTime m_datestarted;
	std::vector<int> m_listensockets;				// sockets we are listening on
	std::vector<IRCClientConnection *> m_clients;
	std::string m_servername;

	std::map<int,idinfo> m_ids;
	std::map<std::string,std::set<int> > m_idchannels;	// channels each id is in
	std::set<int> m_idhassent;							// contains id if that identity has already sent a message within the window - we will accept all messages after no matter when they were sent

	#ifdef _WIN32
	static bool m_wsastartup;
	#endif

};

#endif	// _ircserver_
