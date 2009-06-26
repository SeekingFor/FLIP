#include "freenetmessageeditionpoller.h"
#include "../flipeventsource.h"
#include "../option.h"
#include "../stringfunctions.h"

FreenetMessageEditionPoller::FreenetMessageEditionPoller(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IFCPMessageHandler(connection,"MessageEditionPoller")
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_IDENTITYFOUND,this);
}

FreenetMessageEditionPoller::~FreenetMessageEditionPoller()
{

}

void FreenetMessageEditionPoller::FCPConnected()
{
	Option option;
	option.Get("MessageBase",m_messagebase);
}

void FreenetMessageEditionPoller::FCPDisconnected()
{

}

const bool FreenetMessageEditionPoller::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		m_log->Debug("FreenetMessageEditionPoller::HandleFCPMessage got "+message.GetName());
		return true;
	}
	return false;
}

const bool FreenetMessageEditionPoller::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_IDENTITYFOUND)
	{
		int lastindex=-1;
		std::string indexstr("0");
		std::string publickey("");
		std::map<std::string,std::string> params=flipevent.GetParameters();

		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,params["identityid"]);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,publickey);
		}

		StringFunctions::Convert(params["lastmessageindex"],lastindex);

		// request index+1, index+10, and index+50
		lastindex++;
		StringFunctions::Convert(lastindex,indexstr);
		FCPv2::Message mess("ClientGet");
		mess["URI"]="SSK@"+publickey.substr(4)+m_messagebase+"|"+params["date"]+"|Message-"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+params["identityid"]+"|"+params["date"]+"|"+indexstr;
		mess["ReturnType"]="none";
		m_fcp->Send(mess);

		lastindex+=9;
		StringFunctions::Convert(lastindex,indexstr);
		mess["URI"]="SSK@"+publickey.substr(4)+m_messagebase+"|"+params["date"]+"|Message-"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+params["identityid"]+"|"+params["date"]+"|"+indexstr;
		m_fcp->Send(mess);

		lastindex+=40;
		StringFunctions::Convert(lastindex,indexstr);
		mess["URI"]="SSK@"+publickey.substr(4)+m_messagebase+"|"+params["date"]+"|Message-"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+params["identityid"]+"|"+params["date"]+"|"+indexstr;
		m_fcp->Send(mess);

		m_log->Debug("FreenetMessageEditionPoller::HandleFLIPEvent started looking for new message editions for "+params["identityid"]);

	}
	return false;
}