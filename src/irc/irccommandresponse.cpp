#include "irccommandresponse.h"
#include <sstream>

const IRCCommand IRCCommandResponse::MakeCommand(const std::string &sender, const int responsecode, const std::string &target, const std::string &text)
{
	std::ostringstream ostr;

	ostr.width(3);
	ostr.fill('0');
	ostr << responsecode;

	if(text!="")
	{
		return IRCCommand(":"+sender+" "+ostr.str()+" "+target+" "+text);
	}
	else
	{
		return IRCCommand(":"+sender+" "+ostr.str()+" "+target);
	}
}
