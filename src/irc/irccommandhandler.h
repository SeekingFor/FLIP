#ifndef _irccommandhandler_
#define _irccommandhandler_

#include "irccommand.h"

class IRCClientConnection;

class IRCCommandHandler
{
public:
	virtual const bool HandleCommand(const IRCCommand &command, IRCClientConnection *client)=0;
};

#endif	// _irccommandhandler_
