#ifndef _fcpmessagehandlerregistrar_
#define _fcpmessagehandlerregistrar_

#include <vector>

class IFCPMessageHandler;

class FCPMessageHandlerRegistrar
{
public:
	virtual ~FCPMessageHandlerRegistrar()		{ }

	void Register(IFCPMessageHandler *handler)	{ m_messagehandlers.push_back(handler); }

protected:

	std::vector<IFCPMessageHandler *> m_messagehandlers;

};

#endif	// _fcpmessagehandlerregistrar_
