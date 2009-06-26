#ifndef _fcpconnectedregistrar_
#define _fcpconnectedregistrar_

#include <vector>

class IFCPConnected;

class FCPConnectedRegistrar
{
public:
	virtual ~FCPConnectedRegistrar()			{ }
	
	void Register(IFCPConnected *fcpconnected)	{ m_fcpconnected.push_back(fcpconnected); }

protected:

	std::vector<IFCPConnected *> m_fcpconnected;

};

#endif	// _fcpconnectedregistrar_
