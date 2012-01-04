#ifndef _flipevent_
#define _flipevent_

#include <map>
#include <string>

class FLIPEvent
{
public:
	FLIPEvent(const int type);
	FLIPEvent(const int type, const std::map<std::string,std::string> &parameters);
	~FLIPEvent();
	
	enum EventType
	{
		EVENT_IRC_USERREGISTER=1,
		EVENT_IRC_JOINCHANNEL,
		EVENT_IRC_PARTCHANNEL,
		EVENT_IRC_IGNORE,
		EVENT_IRC_CHANNELMESSAGE,
		EVENT_IRC_PRIVATEMESSAGE,
		EVENT_IRC_USERQUIT,
		EVENT_IRC_KEEPALIVE,
		EVENT_FREENET_CONNECTED,
		EVENT_FREENET_DISCONNECTED,
		EVENT_FREENET_IDENTITYFOUND,
		EVENT_FREENET_IDENTITYINACTIVE,
		EVENT_FREENET_NEWMESSAGEEDITION,
		EVENT_FREENET_NEWCHANNELMESSAGE,
		EVENT_FREENET_NEWPRIVATEMESSAGE,
		EVENT_FREENET_JOINCHANNEL,
		EVENT_FREENET_PARTCHANNEL,
		EVENT_FREENET_KEEPALIVE,
		EVENT_IDENTITYACTIVE
	};

	const int GetType() const										{ return m_type; }
	const std::map<std::string,std::string> &GetParameters() const	{ return m_parameters; }

	const bool operator<(const FLIPEvent &rhs) const				{ return ( (m_type<rhs.m_type) || (m_type==rhs.m_type && m_parameters<rhs.m_parameters) ); }

private:
	
	int m_type;
	std::map<std::string,std::string> m_parameters;

};

#endif	// _flipevent_
