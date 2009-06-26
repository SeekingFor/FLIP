#ifndef _freenetmessageeditionpoller_
#define _freenetmessageeditionpoller_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../flipeventhandler.h"

/*
	Unfortunately, SubscribeUSK isn't very timely in finding new editions, so we need to help it a bit by simply requesting editions that may be there
*/
class FreenetMessageEditionPoller:public IFCPConnected,public IFCPMessageHandler,public IDatabase,public ILogger,public FLIPEventHandler
{
public:
	FreenetMessageEditionPoller(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetMessageEditionPoller();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:
	std::string m_messagebase;

};

#endif	// _freenetmessageindexpoller_
