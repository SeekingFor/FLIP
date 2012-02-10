#ifndef _ircclientthread_
#define _ircclientthread_

#include "../ilogger.h"
#include "ircclientconnection.h"

#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <dlib/threads.h>

class IRCClientThread:public dlib::threaded_object, public ILogger
{
public:
	IRCClientThread(const int sock, const int connectiontype, IRCClientConnection::ssl_client_info *ssl);
	~IRCClientThread();

	const bool IsConnected();
	// disconnect does the disconnection, this will just set a var that indicates we want to disconnect
	const bool SetDisconnect()	{ m_wantdisconnect=true; return true; }

	const bool SendCommand(const IRCCommand &command);
	const bool GetNextCommand(std::string &commandstring);

private:
	void thread();
	const bool Disconnect();
	const bool SetupClientSSL(IRCClientConnection::ssl_client_info &ssl, int socket);
	const int SocketReceive();
	const int SocketSend();
	const std::vector<char>::size_type SendBufferSize() const	{ dlib::auto_mutex guard(m_buffermutex); return m_sendbuffer.size(); }

	IRCClientConnection::ssl_client_info *m_ssl;
	dlib::rmutex m_socketmutex;
	dlib::rmutex m_buffermutex;
	bool m_wantdisconnect;
	int m_socket;
	int m_contype;
	std::vector<char> m_tempbuffer;
	std::vector<char> m_receivebuffer;
	std::vector<char> m_sendbuffer;
};

#endif	// _ircclientthread_
