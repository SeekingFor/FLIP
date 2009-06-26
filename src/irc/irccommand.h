#ifndef _irccommand_
#define _irccommand_

#include <string>
#include <vector>

class IRCCommand
{
public:
	IRCCommand(const std::string &commandstring);

	const std::string &GetCommandString() const				{ return m_commandstring; }
	const std::string &GetPrefix() const					{ return m_prefix; }
	const std::string &GetCommand() const					{ return m_command; }
	const std::vector<std::string> &GetParameters() const	{ return m_parameters; }

private:

	std::string m_prefix;
	std::string m_command;
	std::vector<std::string> m_parameters;
	std::string m_commandstring;			// full command line

};

#endif	// _irccommand_
