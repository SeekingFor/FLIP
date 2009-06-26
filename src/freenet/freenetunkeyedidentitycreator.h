#ifndef _fnunkeyedidentitycreator_
#define _fnunkeyedidentitycreator_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../ilogger.h"
#include "../datetime.h"

#include <ctime>

class FreenetUnkeyedIdentityCreator:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	FreenetUnkeyedIdentityCreator(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetUnkeyedIdentityCreator();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

private:

	void StartSSKRequest(const int localidentityid);

	bool m_waiting;
	DateTime m_lastchecked;

};

#endif	// _fnunkeyedidentitycreator_
