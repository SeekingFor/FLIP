#ifndef _ircchannel_
#define _ircchannel_

#include <string>

class IRCChannel
{
public:
	static const bool IsValid(const std::string &name);
};

#endif	// _ircchannel_
