#ifndef _ifcpmessagehandler_
#define _ifcpmessagehandler_

#include "fcpv2.h"
#include "fcpmessagehandlerregistrar.h"

class IFCPMessageHandler
{
public:
	IFCPMessageHandler(FCPMessageHandlerRegistrar *registrar):m_fcpuniqueidentifier("")	{ registrar->Register(this); }
	IFCPMessageHandler(FCPMessageHandlerRegistrar *registrar, const std::string &uniqueidentifier):m_fcpuniqueidentifier(uniqueidentifier)	{ registrar->Register(this); }
	virtual ~IFCPMessageHandler()								{ }

	virtual const bool HandleFCPMessage(FCPv2::Message &message)=0;

protected:

	std::string m_fcpuniqueidentifier;

};

#endif	// _ifcpmessagehandler_
