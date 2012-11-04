#include "ircchannel.h"
#include "../stringfunctions.h"

std::string IRCChannel::m_prefix="&+#!";

IRCChannel::IRCChannel():m_type(TYPE_STANDARD),m_mode(0),m_topicset(false),m_topic(""),m_name("")
{

}

const bool IRCChannel::SetName(const std::string &name)
{
	if(ValidName(name)==true)
	{
		if(name.find_first_of(m_prefix)==0)
		{
			StringFunctions::LowerCase(name.substr(1),m_name);
			SetType(m_prefix.find(name[0]));
		}
		else
		{
			StringFunctions::LowerCase(name,m_name);
		}
		return true;
	}
	else
	{
		return false;	
	}
}

const bool IRCChannel::SetTopic(const std::string &topic)
{
	if(! ValidTopic(topic))
	{
		return false;
	}
	
	m_topic=topic;
	m_topicset=true;
	
	return true;
}

const bool IRCChannel::ValidName(const std::string &name)
{
	static std::string invalidchars("\x00\x07\x0a\x0d ,:");
	
	return (name.find_first_of(invalidchars)==std::string::npos && name.size()>0);
}

const bool IRCChannel::ValidTopic(const std::string &topic)
{
	static std::string invalidchars("\x00\x07\x0a\x0d");
	
	return (topic.find_first_of(invalidchars)==std::string::npos && topic.size()>0);
}
