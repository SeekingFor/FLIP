#ifndef _freenetnewidentityfinder_
#define _freenetnewidentityfinder_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"

#include <string>

class FreenetNewIdentityFinder:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	FreenetNewIdentityFinder(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetNewIdentityFinder();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

private:
	void StartRequest();

	std::string m_messagebase;
	bool m_waiting;
	DateTime m_lastactivity;
	DateTime m_lastfailed;

};

#endif	// _freenetnewidentityfinder_
