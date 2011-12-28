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

	IRCClientConnection(const int sock, IRCCommandHandler *commandhandler, int connectiontype, ssl_client_info *ssl);

	const bool IsConnected();
	const bool Disconnect();

	const int Socket() const									{ return m_socket; }

	const bool SendCommand(const IRCCommand &command);
	const std::vector<char>::size_type SendBufferSize() const	{ return m_sendbuffer.size(); }

	const int SocketReceive();
	const int SocketSend();

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
	int &DBID()									{ return m_dbid; }
	const int DBID() const						{ return m_dbid; }
	std::string &PublicKey()					{ return m_publickey; }
	const std::string &PublicKey() const		{ return m_publickey; }
	std::string &RSAPrivateKey()				{ return m_rsaprivatekey; }
	const std::string &RSAPrivateKey() const	{ return m_rsaprivatekey; }
	DateTime &LastActivity()					{ return m_lastactivity; }
	const DateTime &LastActivity() const		{ return m_lastactivity; }

private:
	const bool HandleReceivedData();

	IRCCommandHandler *m_commandhandler;
	int m_socket;
	std::vector<char> m_tempbuffer;
	std::vector<char> m_receivebuffer;
	std::vector<char> m_sendbuffer;
	std::set<std::string> m_joinedchannels;

	int m_dbid;
	int m_registered;
	std::string m_nick;
	std::string m_user;
	std::string m_host;
	std::string m_realname;
	std::string m_publickey;
	std::string m_rsaprivatekey;
	DateTime m_lastactivity;
	int m_contype;					// type of connection - secure or SSL
	ssl_client_info *m_ssl;
};

#endif	// _ircclientconnection_
