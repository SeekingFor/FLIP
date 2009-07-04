#ifndef _freenetmessageeditionpoller_
#define _freenetmessageeditionpoller_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../flipeventhandler.h"
#include "../datetime.h"

#include <map>

/*
	Unfortunately, SubscribeUSK isn't very timely in finding new editions, so we need to help it a bit by simply requesting editions that may be there
*/
class FreenetMessageEditionPoller:public IFCPConnected,public IFCPMessageHandler,public IDatabase,public ILogger,public FLIPEventHandler,public IPeriodicProcessor
{
public:
	FreenetMessageEditionPoller(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetMessageEditionPoller();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

	void Process();

private:
	void RequestMessageEdition(const DateTime &date, const int identityid, const int edition);

	std::string m_messagebase;

	struct idinfo
	{
		std::string m_publickey;
		DateTime m_lastactivity;
	};

	std::map<int,idinfo> m_ids;
	std::map<DateTime,std::map<int,int> > m_lastmessageedition;	// date,id,edition#

};

#endif	// _freenetmessageindexpoller_
