#include "irccommand.h"
#include "../stringfunctions.h"

#include <vector>

IRCCommand::IRCCommand(const std::string &commandstring)
{
	std::vector<std::string> commandparts;
	std::vector<std::string>::size_type pos=0;
	std::string textparam("");
	bool intextparam=false;

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
		// start of text param :
		if(intextparam==false && commandparts[pos].size()>0 && commandparts[pos][0]==':')
		{
			intextparam=true;
			textparam=commandparts[pos];
		}
		// end of text param :
		else if(intextparam==true && commandparts[pos].size()>0 && commandparts[pos][commandparts[pos].size()-1]==':')
		{
			textparam+=" "+commandparts[pos];
			m_parameters.push_back(textparam);
			intextparam=false;
		}
		// middle of text param
		else if(intextparam==true)
		{
			textparam+=" "+commandparts[pos];
		}
		// regular param
		else
		{
			m_parameters.push_back(commandparts[pos]);
		}
		pos++;
	}

	if(intextparam==true)
	{
		m_parameters.push_back(textparam);
	}

}
