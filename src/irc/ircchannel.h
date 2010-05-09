#ifndef _ircchannel_
#define _ircchannel_

#include <string>

class IRCChannel
{
public:
	IRCChannel();

	enum ChannelType
	{
		TYPE_LOCAL=0,		// &
		TYPE_NOMODE,		// +
		TYPE_STANDARD,		// #
		TYPE_SAFE,			// !
	};
	
	enum ChannelMode
	{
		MODE_GIVE_CHANNEL_CREATOR=1,				// O
		MODE_GIVE_TAKE_CHANNEL_OPERATOR=2,			// o
		MODE_GIVE_TAKE_VOICE=4,						// v
		MODE_TOGGLE_ANONYMOUS=8,					// a
		MODE_TOGGLE_INVITE_ONLY=16,					// i
		MODE_TOGGLE_MODERATED=32,					// m
		MODE_TOGGLE_NO_OUTSITE_MESSAGES=64,			// n
		MODE_TOGGLE_QUIET=128,						// q
		MODE_TOGGLE_PRIVATE=256,					// p
		MODE_TOGGLE_SECRET=512,						// s
		MODE_TOGGLE_REOP=1024,						// r
		MODE_TOGGLE_TOPIC_SETTABLE=2048,			// t
		MODE_SET_REMOVE_KEY=4096,					// k
		MODE_SET_REMOVE_USER_LIMIT=8192,			// l
		MODE_SET_REMOVE_BAN_MASK=16384,				// b
		MODE_SET_REMOVE_EXCEPTION_MASK=32768,		// e
		MODE_SET_REMOVE_INVITATION_MASK=65536		// I
	};
	
	void SetType(const int type)			{ type>=0 && type<4 ? m_type=type : false; }
	const int GetType() const				{ return m_type; }
	void SetMode(const int mode)			{ m_mode|=mode; }
	void UnsetMode(const int mode)			{ m_mode&=(~mode); }
	const int GetMode() const				{ return m_mode; }
	const bool GetTopicSet() const			{ return m_topicset; }
	void SetTopic(const std::string &topic)	{ m_topic=topic; m_topicset=true; }
	const std::string &GetTopic() const		{ return m_topic; }
	void UnsetTopic()						{ m_topic=""; m_topicset=false; }
	/* May also set type if there is a prefix on the name */
	const bool SetName(const std::string &name);
	const std::string GetName(const bool withprefix=true) const		{ return withprefix==true ? m_prefix[m_type]+m_name : m_name; }
	const char GetPrefix() const;
	
	static const bool ValidName(const std::string &name);
	
	const bool operator==(const IRCChannel &rhs)	{ return GetName(true)==rhs.GetName(true); }
	const bool operator<(const IRCChannel &rhs)		{ return GetName(true)<rhs.GetName(true); }

private:

	static std::string m_prefix;		// prefixes of channel names

	int m_type;
	int m_mode;
	bool m_topicset;
	std::string m_topic;
	std::string m_name;

};

#endif	// _ircchannel_
