#ifndef _freenetmessagedownloader_
#define _freenetmessagedownloader_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "../flipeventhandler.h"
#include "../flipeventsource.h"
#include "../idatabase.h"
#include "../ilogger.h"

class FreenetMessageDownloader:public IFCPConnected,public IFCPMessageHandler,public FLIPEventHandler,public FLIPEventSource,public IDatabase,public ILogger
{
public:
	FreenetMessageDownloader(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetMessageDownloader();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:

	void StartRequest(const int identityid, const std::string &publickey, const std::string &date, const int edition);

	std::string m_messagebase;

};

#endif	// _freenetmessagedownloader_
