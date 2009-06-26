#include "irccommand.h"
#include "../stringfunctions.h"

#include <vector>

IRCCommand::IRCCommand(const std::string &commandstring)
{
	std::vector<std::string> commandparts;
	std::vector<std::string>::size_type pos=0;

	m_commandstring=commandstring;
	StringFunctions::Split(m_commandstring," ",commandparts);

	// prefix
	if(commandparts.size()>pos && commandparts[pos][0]==':')
	{
		m_prefix=commandparts[pos];
		pos++;
	}

	// command
	if(commandparts.size()>pos)
	{
		StringFunctions::UpperCase(commandparts[pos],m_command);
		pos++;
	}

	// parameters
	while(pos<commandparts.size())
	{
		m_parameters.push_back(commandparts[pos]);
		pos++;
	}

}
