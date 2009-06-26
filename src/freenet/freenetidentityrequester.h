#ifndef _freenetidentityrequester_
#define _freenetidentityrequester_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"
#include "../flipeventsource.h"

#include <vector>

class FreenetIdentityRequester:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger,public FLIPEventSource
{
public:
	FreenetIdentityRequester(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetIdentityRequester();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

private:
	void StartRequest(const int identityid, const int edition=-1);
	void RemoveRequest(const int identityid);

	std::string m_messagebase;
	DateTime m_lastactivity;
	DateTime m_lastloadedids;
	std::set<int> m_ids;
	std::set<int> m_requesting;
	std::set<int>::size_type m_maxrequests;

};

#endif	// _freenetidentityrequester_
