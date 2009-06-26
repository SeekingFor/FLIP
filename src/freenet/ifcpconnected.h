#ifndef _ifcpconnected_
#define _ifcpconnected_

#include "fcpv2.h"
#include "fcpconnectedregistrar.h"

class IFCPConnected
{
public:
	IFCPConnected(FCPv2::Connection *fcp, FCPConnectedRegistrar *registrar):m_fcp(fcp)	{ registrar->Register(this); }
	virtual ~IFCPConnected()															{ }

	virtual void FCPConnected()=0;
	virtual void FCPDisconnected()=0;

protected:
	FCPv2::Connection *m_fcp;
};

#endif	// _ifcpconnected_
