#include "ircchannel.h"

const bool IRCChannel::IsValid(const std::string &name)
{
	static const std::string invalidchars("\x00\x07\x0a\x0d ,:");

	return (name.find_first_of(invalidchars)==std::string::npos);
}
