#ifndef _ircclientconnection_
#define _ircclientconnection_

#include <vector>
#include <set>

#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>

#include "irccommandhandler.h"
#include "../ilogger.h"
#include "../datetime.h"

class IRCClientThread;

class IRCClientConnection:public ILogger
{
public:

	struct ssl_client_info
	{
		entropy_context m_entropy;
		ctr_drbg_context m_ctr_drbg;
		ssl_context m_ssl;
		ssl_session m_session;
	};

	IRCClientConnection(const int sock, IRCCommandHandler *commandhandler, const int connectiontype, ssl_client_info *ssl);
	~IRCClientConnection();

	const bool IsConnected();
	const bool Disconnect();

	const bool SendCommand(const IRCCommand &command);
	const bool HandleReceivedData();

	enum registered
	{
		REG_NICK=1,
		REG_USER=2
	};

	enum con_type
	{
		CON_UNSECURE=1,
		CON_SSL=2
	};

	int &Registered()							{ return m_registered; }
	const int Registered() const				{ return m_registered; }

	std::string &Nick()							{ return m_nick; }
	const std::string &Nick() const				{ return m_nick; }
	std::string &User()							{ return m_user; }
	const std::string &User() const				{ return m_user; }
	std::string &Host()							{ return m_host; }
	const std::string &Host() const				{ return m_host; }
	std::string &RealName()						{ return m_realname; }
	const std::string &RealName() const			{ return m_realname; }
	const std::set<std::string> &JoinedChannels() const	{ return m_joinedchannels; }
	std::set<std::string> &JoinedChannels()				{ return m_joinedchannels; }
	int &LocalDBID()							{ return m_localdbid; }
	const int LocalDBID() const					{ return m_localdbid; }
	int &PeerDBID()								{ return m_peerdbid; }
	const int PeerDBID() const					{ return m_peerdbid; }
	std::string &PublicKey()					{ return m_publickey; }
	const std::string &PublicKey() const		{ return m_publickey; }
	std::string &RSAPrivateKey()				{ return m_rsaprivatekey; }
	const std::string &RSAPrivateKey() const	{ return m_rsaprivatekey; }
	DateTime &LastActivity()					{ return m_lastactivity; }
	const DateTime &LastActivity() const		{ return m_lastactivity; }

private:
	IRCCommandHandler *m_commandhandler;
	std::set<std::string> m_joinedchannels;

	int m_localdbid;
	int m_peerdbid;
	int m_registered;
	std::string m_nick;
	std::string m_user;
	std::string m_host;
	std::string m_realname;
	std::string m_publickey;
	std::string m_rsaprivatekey;
	DateTime m_lastactivity;
	IRCClientThread *m_clientthread;
};

#endif	// _ircclientconnection_
