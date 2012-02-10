#include "ircclientthread.h"
#include "../stringfunctions.h"

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

IRCClientThread::IRCClientThread(const int sock, const int connectiontype, IRCClientConnection::ssl_client_info *ssl):m_socket(sock),m_contype(connectiontype),m_ssl(ssl),m_tempbuffer(4096,0)
{
	m_wantdisconnect=false;
	//we need to set the ssl socket handlers here because of the local socket variable
	if(m_contype==IRCClientConnection::CON_SSL)
	{
		ssl_set_bio(&(ssl->m_ssl),net_recv,&m_socket,net_send,&m_socket);
	}
}

IRCClientThread::~IRCClientThread()
{
	Disconnect();
}

void IRCClientThread::thread()
{
	int ciphersuites[] =
	{
		SSL_EDH_RSA_AES_256_SHA,
		SSL_EDH_RSA_CAMELLIA_256_SHA,
		SSL_EDH_RSA_AES_128_SHA,
		SSL_EDH_RSA_CAMELLIA_128_SHA,
		SSL_EDH_RSA_DES_168_SHA,
		SSL_RSA_AES_256_SHA,
		SSL_RSA_CAMELLIA_256_SHA,
		SSL_RSA_AES_128_SHA,
		SSL_RSA_CAMELLIA_128_SHA,
		SSL_RSA_DES_168_SHA,
		SSL_RSA_RC4_128_SHA,
		SSL_RSA_RC4_128_MD5,
		0
	};
	int rval=0;
	fd_set readfs;
	fd_set writefs;
	struct timeval tv;
	std::string temp("");

	// setup SSL connection first
	if(m_contype==IRCClientConnection::CON_SSL)
	{
		m_log->Error("IRCClientThread::thread SSL handshaking with client");
		ssl_set_ciphersuites(&(m_ssl->m_ssl),ciphersuites);
		rval=ssl_handshake(&(m_ssl->m_ssl));
		if(rval!=0)
		{
			StringFunctions::Convert(rval,temp);
			m_log->Error("IRCClientThread::thread couldn't handshake with client - return value = "+temp);
			Disconnect();
			return;
		}
	}

	while(should_stop()==false && IsConnected()==true)
	{
		tv.tv_sec=0;
		tv.tv_usec=100000;
		FD_ZERO(&readfs);
		FD_ZERO(&writefs);

		FD_SET(m_socket,&readfs);
		if(SendBufferSize()>0)
		{
			FD_SET(m_socket,&writefs);
		}

		rval=select(m_socket+1,&readfs,&writefs,0,&tv);

		if(rval>0)
		{
			if(FD_ISSET(m_socket,&readfs))
			{
				SocketReceive();
			}
			if(IsConnected() && FD_ISSET(m_socket,&writefs))
			{
				SocketSend();
			}
		}

		if(m_wantdisconnect==true)
		{
			Disconnect();
		}
	}
}

const bool IRCClientThread::Disconnect()
{
	dlib::auto_mutex guard(m_socketmutex);
	if(IsConnected())
	{
		#ifdef _WIN32
		closesocket(m_socket);
		#else
		close(m_socket);
		#endif
		m_socket=-1;
	}
	if(m_ssl)
	{
		ssl_free(&m_ssl->m_ssl);
		delete m_ssl;
		m_ssl=0;
	}
	m_wantdisconnect=false;
	return true;
}

const bool IRCClientThread::GetNextCommand(std::string &commandstring)
{
	dlib::auto_mutex guard(m_buffermutex);
	std::vector<char> commandend(2,0);
	commandend[0]='\r';
	commandend[1]='\n';
	std::vector<char>::iterator endpos=m_receivebuffer.end();
	std::vector<char>::size_type endsize=1;

	if(m_receivebuffer.size()>0 && (endpos=std::find(m_receivebuffer.begin(),m_receivebuffer.end(),commandend[1]))!=m_receivebuffer.end())
	{
		if(endpos>m_receivebuffer.begin() && *(endpos-1)==commandend[0])
		{
			endpos--;
			endsize+=1;
		}
		commandstring=std::string(m_receivebuffer.begin(),endpos);
		m_receivebuffer.erase(m_receivebuffer.begin(),endpos+endsize);
		return true;
	}
	else
	{
		return false;
	}
}

const bool IRCClientThread::IsConnected()
{
	dlib::auto_mutex guard(m_socketmutex);
	return m_socket!=-1 ? true : false ;
}

const bool IRCClientThread::SendCommand(const IRCCommand &command)
{
	dlib::auto_mutex guard(m_buffermutex);
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

const int IRCClientThread::SocketReceive()
{
	dlib::auto_mutex guard(m_buffermutex);
	if(IsConnected() && m_contype==IRCClientConnection::CON_UNSECURE)
	{
		int rval=recv(m_socket,&m_tempbuffer[0],m_tempbuffer.size(),0);
		if(rval>0)
		{
			m_receivebuffer.insert(m_receivebuffer.end(),m_tempbuffer.begin(),m_tempbuffer.begin()+rval);
			//while(HandleReceivedData())
			{
			}
		}
		else if(rval==0)
		{
			Disconnect();
			m_log->Info("IRCClientThread::SocketReceive remote host closed connection");
		}
		else if(rval==-1)
		{
			Disconnect();
			m_log->Error("IRCClientThread::SocketReceive recv returned -1.  Closing connection.");
		}
		return rval;
	}
	else if(IsConnected() && m_contype==IRCClientConnection::CON_SSL)
	{
		int rval=ssl_read(&(m_ssl->m_ssl),(unsigned char *)&m_tempbuffer[0],m_tempbuffer.size());
		if(rval>0)
		{
			m_receivebuffer.insert(m_receivebuffer.end(),m_tempbuffer.begin(),m_tempbuffer.begin()+rval);
			//while(HandleReceivedData())
			{
			}
		}
		else if(rval==0)
		{
			Disconnect();
			m_log->Error("IRCClientThread::SocketReceive ssl_read returned 0.  Closing connection.");
		}
		else if(rval<0)
		{
			Disconnect();
			m_log->Error("IRCClientThread::SocketReceive ssl_read returned < 0.  Closing connection.");
		}
		return rval;
	}
	else
	{
		return -1;
	}
}

const int IRCClientThread::SocketSend()
{
	dlib::auto_mutex guard(m_buffermutex);
	if(IsConnected() && m_sendbuffer.size()>0 && m_contype==IRCClientConnection::CON_UNSECURE)
	{
		int rval=send(m_socket,&m_sendbuffer[0],m_sendbuffer.size(),0);
		if(rval>0)
		{
			m_sendbuffer.erase(m_sendbuffer.begin(),m_sendbuffer.begin()+rval);
		}
		else if(rval==-1)
		{
			m_log->Error("IRCClientThread::SocketSend returned -1");
		}
		return rval;
	}
	else if(IsConnected() && m_sendbuffer.size()>0 && m_contype==IRCClientConnection::CON_SSL)
	{
		int rval=ssl_write(&(m_ssl->m_ssl),(unsigned char *)&m_sendbuffer[0],m_sendbuffer.size());
		if(rval>0)
		{
			m_sendbuffer.erase(m_sendbuffer.begin(),m_sendbuffer.begin()+rval);
		}
		else
		{
			m_log->Error("IRCClientThread::SocketSend ssl_write returned <= 0.");
		}
		return rval;
	}
	else
	{
		return -1;
	}
}
