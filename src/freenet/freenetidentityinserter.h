#ifndef _freenetidentityinserter_
#define _freenetidentityinserter_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../flipeventhandler.h"
#include "../datetime.h"

#include <map>
#include <string>

class FreenetIdentityInserter:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger,public FLIPEventHandler
{
public:
	FreenetIdentityInserter(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetIdentityInserter();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:

	void StartIDInsert(const int id);

	struct idinfo
	{
		DateTime m_lastinserted;
		std::string m_privatekey;
		std::string m_name;
	};

	std::string m_messagebase;
	std::map<int,idinfo> m_activeids;
	bool m_inserting;
	DateTime m_lastactivity;

};

#endif	// _freenetidentityinserter_
