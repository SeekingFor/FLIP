#include "ircclientconnection.h"
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

IRCClientConnection::IRCClientConnection(const int sock, IRCCommandHandler *commandhandler):m_socket(sock)
,m_commandhandler(commandhandler),m_tempbuffer(4096,0),m_registered(0),m_nick(""),m_user(""),m_realname(""),m_host("freenet"),m_publickey(""),m_rsaprivatekey("")
{
	m_lastactivity.SetNowUTC();
}

const bool IRCClientConnection::Disconnect()
{
	if(IsConnected())
	{
		#ifdef _WIN32
		closesocket(m_socket);
		#else
		close(m_socket);
		#endif
		m_socket=-1;
	}
	return true;
}

const bool IRCClientConnection::HandleReceivedData()
{
	std::vector<char> commandend(2,0);
	commandend[0]='\r';
	commandend[1]='\n';
	std::vector<char>::iterator endpos=m_receivebuffer.end();
	std::vector<char>::size_type endsize=1;

	if(m_receivebuffer.size()>0 && (endpos=std::find(m_receivebuffer.begin(),m_receivebuffer.end(),commandend[1]))!=m_receivebuffer.end())
	//if(m_receivebuffer.size()>0 && (endpos=std::search(m_receivebuffer.begin(),m_receivebuffer.end(),commandend.begin(),commandend.end()))!=m_receivebuffer.end())
	{
		if(endpos>m_receivebuffer.begin() && *(endpos-1)==commandend[0])
		{
			endpos--;
			endsize+=1;
		}
		m_commandhandler->HandleCommand(IRCCommand(std::string(m_receivebuffer.begin(),endpos)),this);
		m_receivebuffer.erase(m_receivebuffer.begin(),endpos+endsize);
		return true;
	}
	else
	{
		return false;
	}
}

const bool IRCClientConnection::IsConnected()
{
	return m_socket!=-1 ? true : false ;
}

const bool IRCClientConnection::SendCommand(const IRCCommand &command)
{
	if(IsConnected())
	{
		m_sendbuffer.insert(m_sendbuffer.end(),command.GetCommandString().begin(),command.GetCommandString().end());
		m_sendbuffer.push_back('\r');
		m_sendbuffer.push_back('\n');
		return true;
	}
	else
	{
		return false;
	}
}

const int IRCClientConnection::SocketReceive()
{
	if(IsConnected())
	{
		int rval=recv(m_socket,&m_tempbuffer[0],m_tempbuffer.size(),0);
		if(rval>0)
		{
			m_receivebuffer.insert(m_receivebuffer.end(),m_tempbuffer.begin(),m_tempbuffer.begin()+rval);
			while(HandleReceivedData())
			{
			}
		}
		else if(rval==0)
		{
			Disconnect();
			m_log->Info("IRCClientConnection::SocketReceive remote host closed connection");
		}
		else if(rval==-1)
		{
			Disconnect();
			m_log->Error("IRCClientConnection::SocketReceive recv returned -1.  Closing connection.");
		}
		return rval;
	}
	else
	{
		return -1;
	}
}

const int IRCClientConnection::SocketSend()
{
	if(IsConnected() && m_sendbuffer.size()>0)
	{
		int rval=send(m_socket,&m_sendbuffer[0],m_sendbuffer.size(),0);
		if(rval>0)
		{
			m_sendbuffer.erase(m_sendbuffer.begin(),m_sendbuffer.begin()+rval);
		}
		else if(rval==-1)
		{
			m_log->Error("IRCClientConnection::SocketSend returned -1");
		}
		return rval;
	}
	else
	{
		return -1;
	}
}