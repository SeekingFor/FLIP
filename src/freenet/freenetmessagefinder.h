#ifndef _freenetmessagefinder_
#define _freenetmessagefinder_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../flipeventsource.h"
#include "../flipeventhandler.h"
#include "../datetime.h"

#include <map>
#include <set>

class FreenetMessageFinder:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger,public FLIPEventHandler,public FLIPEventSource
{
public:
	FreenetMessageFinder(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetMessageFinder();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:
	void StartRequests();
	void Subscribe(const int identityid, const std::string &publickey, const DateTime date, const int edition=-1);
	void Unsubscribe(const int identityid, const DateTime date);

	struct idinfo
	{
		DateTime m_lastactive;
	};

	std::string m_messagebase;
	std::map<int,idinfo> m_ids;
	std::map<DateTime,std::set<int> > m_subscribed;

};

#endif	// _freenetmessagefinder_
