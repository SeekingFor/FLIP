#ifndef _ircserver_
#define _ircserver_

#include "../ilogger.h"
#include "../idatabase.h"
#include "../flipeventsource.h"
#include "../flipeventhandler.h"
#include "../datetime.h"
#include "ircclientconnection.h"
#include "irccommandhandler.h"
#include "ircchannel.h"

#include <polarssl/ssl.h>

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
	void SendTopicToClients(const int identityid, const std::string &channel, const std::string &topic);
	void SendPrivateMessageToClients(const int identityid, const std::string &recipient, const std::string &encryptedmessage);
	void SendJoinMessageToClients(const int identityid, const std::string &channel);
	void SendPartMessageToClients(const int identityid, const std::string &channel);
	void SendMOTDLines(IRCClientConnection *client);

	void ProcessFLIPEvent(const int identityid, const FLIPEvent &flipevent);
	const bool SetupServerSSL();
	void ShutdownServerSSL();
	const bool SetupClientSSL(IRCClientConnection::ssl_client_info &ssl, int socket);
	void ReloadMOTD();
	const bool NickInUse(IRCClientConnection *client, const std::string &nick) const;

	const bool GetPeerDBID(IRCClientConnection *client);
	void GetPublicKey(IRCClientConnection *client);
	IRCChannel* GetChannel(const std::string&); 
	void dbAddChannel(IRCChannel* chan) const;
	void dbUpdateChannel(IRCChannel* chan) const;
	void dbInitChannels();

	struct idinfo
	{
		struct messagequeueitem
		{
			messagequeueitem(const FLIPEvent &message, const int edition, const DateTime &insertday, const DateTime &arrivaltime):m_message(message),m_edition(edition),m_insertday(insertday),m_arrivaltime(arrivaltime)	{}
			FLIPEvent m_message;
			int m_edition;
			DateTime m_insertday;
			DateTime m_arrivaltime;
		};

		// custom comparator so messages are sorted by earliest messages first
		class messagequeuecompare
		{
		public:
			const bool operator()(const messagequeueitem &lhs, const messagequeueitem &rhs)
			{
				return ( (lhs.m_insertday<rhs.m_insertday) || (lhs.m_insertday==rhs.m_insertday && lhs.m_edition<rhs.m_edition) || (lhs.m_insertday==rhs.m_insertday && lhs.m_edition==rhs.m_edition && lhs.m_message<rhs.m_message) );
			}
		};

		std::string m_nick;
		std::string m_publickey;
		std::map<DateTime,int> m_lastdayedition;	// map of day, and edition # that we already processed from the identity - messages more than +1 after the last edition will be queued
		std::set<messagequeueitem,messagequeuecompare> m_messagequeue;
		DateTime m_lastircactivity;
	};

	DateTime m_datestarted;
	std::vector<int> m_listensockets;				// sockets we are listening on
	std::vector<int> m_ssllistensockets;			// SSL sockets we are listening on
	std::vector<IRCClientConnection *> m_clients;
	std::string m_servername;

	std::map<int,idinfo> m_ids;
	std::map<std::string,std::set<int> > m_idchannels;	// channels each id is in
	std::set<int> m_idhassent;							// contains id if that identity has already sent a message within the window - we will accept all messages after no matter when they were sent
	std::map<std::string, IRCChannel*> m_channels;					// known channels

	std::vector<std::string> m_motdlines;
	struct ssl_server_info
	{
		x509_cert m_cert;
		rsa_context m_rsa;
	};
	ssl_server_info m_ssl;
	bool m_sslsetup;

	#ifdef _WIN32
	static bool m_wsastartup;
	#endif

};

#endif	// _ircserver_
