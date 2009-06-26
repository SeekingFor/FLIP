#include "freenetmessage.h"
#include "../stringfunctions.h"

#include <vector>

FreenetMessage::FreenetMessage()
{

}

FreenetMessage::FreenetMessage(const std::map<std::string,std::string> &headers, const std::string &body)
{

}

FreenetMessage::~FreenetMessage()
{

}

const std::string FreenetMessage::GetMessageText() const
{
	std::string messagetext("");
	for(std::map<std::string,std::string>::const_iterator i=m_headers.begin(); i!=m_headers.end(); i++)
	{
		messagetext+=(*i).first+"="+(*i).second+"\n";
	}
	messagetext+="\n";
	messagetext+=m_body;

	return messagetext;
}

const bool FreenetMessage::TryParse(const std::string &messagetext, FreenetMessage &message)
{
	bool inheaders=true;
	int bodyline=0;
	std::vector<std::string> messagelines;
	std::map<std::string,std::string> headers;
	std::string body("");

	StringFunctions::Split(messagetext,"\n",messagelines);

	for(std::vector<std::string>::iterator i=messagelines.begin(); i!=messagelines.end(); i++)
	{
		if((*i).size()==0)
		{
			inheaders=false;
		}
		else if(inheaders==true)
		{
			std::string::size_type pos=(*i).find("=");
			if(pos!=std::string::npos)
			{
				headers[(*i).substr(0,pos)]=(*i).substr(pos+1);
			}
			else
			{
				return false;
			}
		}
		else
		{
			if(bodyline>0)
			{
				body+="\n";
			}
			body+=(*i);
			bodyline++;
		}
	}

	message.Headers()=headers;
	message.Body()=body;

	return true;

}
