#ifndef _ircnick_
#define _ircnick_

#include <string>

class IRCNick
{
public:
	static const bool IsValid(const std::string &nick);
};

#endif	// _ircnick_
