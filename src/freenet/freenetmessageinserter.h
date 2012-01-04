#ifndef _freenetmessageinserter_
#define _freenetmessageinserter_

#include "freenetconnection.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"
#include "../flipeventhandler.h"
#include "../idatabase.h"
#include "../ilogger.h"

#include <string>
#include <map>

class FreenetMessageInserter:public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger,public FLIPEventHandler
{
public:
	FreenetMessageInserter(FreenetConnection *connection, FCPv2::Connection *fcp);
	~FreenetMessageInserter();

	void FCPConnected();
	void FCPDisconnected();
	const bool HandleFCPMessage(FCPv2::Message &message);
	void Process();

	const bool HandleFLIPEvent(const FLIPEvent &flipevent);

private:
	void StartChannelInsert(const int localidentityid, const std::string &channel, const std::string &message);
	void StartPrivateInsert(const int localidentityid, const int recipientidentityid, const std::string &message);
	void StartJoinChannelInsert(const int localidentityid, const std::string &channel);
	void StartPartChannelInsert(const int localidentityid, const std::string &channel);
	void StartKeepAliveInsert(const int localidentityid, const std::string &channels);
	void LoadLocalIdentityPrivateKey(const int localidentityid);
	const int GetNextMessageIndex(const int localidentityid, const DateTime &date);

	std::string m_messagebase;
	std::map<int,std::string> m_identitykeys;
	std::string m_insertpriority;

};

#endif	// _freenetmessageinserter_
