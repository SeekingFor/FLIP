#ifndef _ircclientconnection_
#define _ircclientconnection_

#include <vector>
#include <set>

#include "irccommandhandler.h"
#include "../ilogger.h"

class IRCClientConnection:public ILogger
{
public:
	IRCClientConnection(const int sock, IRCCommandHandler *commandhandler);

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

	int &Registered()						{ return m_registered; }
	const int Registered() const			{ return m_registered; }

	std::string &Nick()						{ return m_nick; }
	const std::string &Nick() const			{ return m_nick; }
	std::string &User()						{ return m_user; }
	const std::string &User() const			{ return m_user; }
	std::string &Host()						{ return m_host; }
	const std::string &Host() const			{ return m_host; }
	std::string &RealName()					{ return m_realname; }
	const std::string &RealName() const		{ return m_realname; }
	const std::set<std::string> &JoinedChannels() const	{ return m_joinedchannels; }
	std::set<std::string> &JoinedChannels()				{ return m_joinedchannels; }
	int &DBID()								{ return m_dbid; }
	const int DBID() const					{ return m_dbid; }
	std::string &PublicKey()				{ return m_publickey; }
	const std::string &PublicKey() const	{ return m_publickey; }

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
};

#endif	// _ircclientconnection_
