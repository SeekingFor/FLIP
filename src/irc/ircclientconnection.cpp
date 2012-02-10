#include "ircclientconnection.h"
#include "ircclientthread.h"
#include <algorithm>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>  // gcc - IPPROTO_ consts
	#include <netdb.h>       // gcc - addrinfo
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
#endif

IRCClientConnection::IRCClientConnection(const int sock, IRCCommandHandler *commandhandler, const int connectiontype, ssl_client_info *ssl):
m_commandhandler(commandhandler),m_registered(0),m_nick(""),m_user(""),m_realname(""),m_host("freenet"),m_publickey(""),m_rsaprivatekey(""),m_localdbid(-1),m_peerdbid(-1)
{
	m_lastactivity.SetNowUTC();
	m_clientthread=new IRCClientThread(sock,connectiontype,ssl);
	if(m_clientthread)
	{
		m_clientthread->start();
	}
}

IRCClientConnection::~IRCClientConnection()
{
	if(m_clientthread)
	{
		m_clientthread->stop();
		m_clientthread->wait();
		delete m_clientthread;
		m_clientthread=0;
	}
}

const bool IRCClientConnection::Disconnect()
{
	if(m_clientthread)
	{
		return m_clientthread->SetDisconnect();
	}
	return true;
}

const bool IRCClientConnection::HandleReceivedData()
{
	if(m_clientthread)
	{
		std::string commandstring("");
		while(m_clientthread->GetNextCommand(commandstring))
		{
			m_commandhandler->HandleCommand(IRCCommand(commandstring),this);
		}
		return true;
	}
	return false;
}

const bool IRCClientConnection::IsConnected()
{
	if(m_clientthread)
	{
		return m_clientthread->IsConnected();
	}
	return false;
}

const bool IRCClientConnection::SendCommand(const IRCCommand &command)
{
	if(m_clientthread)
	{
		return m_clientthread->SendCommand(command);
	}
	return false;
}
