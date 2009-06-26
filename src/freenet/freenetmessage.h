#ifndef _freenetmessage_
#define _freenetmessage_

#include <map>
#include <string>

class FreenetMessage
{
public:
	FreenetMessage();
	FreenetMessage(const std::map<std::string,std::string> &headers, const std::string &body);
	~FreenetMessage();

	std::map<std::string,std::string> &Headers()	{ return m_headers; }
	std::string &Body()								{ return m_body; }

	std::string &operator[](const std::string &header)	{ return m_headers[header]; }

	const std::string GetMessageText() const;
	
	static const bool TryParse(const std::string &messagetext, FreenetMessage &message);

private:
	std::map<std::string,std::string> m_headers;
	std::string m_body;
};

#endif	// _freenetmessage_
