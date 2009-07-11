#include "ircnick.h"

const bool IRCNick::IsValid(const std::string &nick)
{
	static std::string beginchars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[]\'_^{|}");
	static std::string trailingchars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[]\'_^{|}0123456789-");

	// RFC 2812 says nick can't be longer than 9 chars
	// allow up to 15 for now
	if(nick.size()>0 && nick.size()<16)
	{
		if(beginchars.find(nick[0])==std::string::npos)
		{
			return false;
		}
		for(std::string::size_type pos=1; pos<nick.size(); pos++)
		{
			if(trailingchars.find(nick[pos])==std::string::npos)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}

}
