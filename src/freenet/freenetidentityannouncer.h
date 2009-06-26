#ifndef _freenetidentityannouncer_
#define _freenetidentityannouncer_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../flipeventhandler.h"
#include "../datetime.h"

class FreenetIdentityAnnouncer:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger,public FLIPEventHandler
{
public:
	FreenetIdentityAnnouncer(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetIdentityAnnouncer();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:
	void StartInsert(const int localidentityid);

	struct idinfo
	{
		std::string m_publickey;
		bool m_active;
		bool m_announcing;
		DateTime m_lastannounced;
	};

	std::string m_messagebase;
	std::map<int,idinfo> m_ids;
	DateTime m_lastactivity;


};

#endif	// _freenetidentityannouncer_
