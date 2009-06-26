#ifndef _freenetconnection_
#define _freenetconnection_

#include "fcpv2.h"
#include "fcpconnectedregistrar.h"
#include "fcpmessagehandlerregistrar.h"
#include "periodicprocessorregistrar.h"
#include "../ilogger.h"
#include "../idatabase.h"
#include "../flipeventsource.h"
#include "../datetime.h"

#include <ctime>
#include <vector>

/*

IRCServer -> FreenetConnection
JoinChannel
PartChannel
ChannelMessage

FreenetConnection -> IRCServer
Disconnected
Connected

*/

class FreenetConnection:public ILogger,public IDatabase,public PeriodicProcessorRegistrar,public FCPConnectedRegistrar,public FCPMessageHandlerRegistrar,public FLIPEventSource
{
public:
	FreenetConnection();
	~FreenetConnection();

	void Update(const unsigned long ms);

private:

	const bool Connect();

	FCPv2::Connection m_fcp;
	bool m_receivednodehello;
	bool m_wasconnected;
	DateTime m_lasttriedconnecting;

	std::vector<void *> m_objects;

};

#endif	// _freenetconnection_
