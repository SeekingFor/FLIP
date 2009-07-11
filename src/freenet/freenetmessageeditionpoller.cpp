#include "freenetmessageeditionpoller.h"
#include "../flipeventsource.h"
#include "../option.h"
#include "../stringfunctions.h"

FreenetMessageEditionPoller::FreenetMessageEditionPoller(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IFCPMessageHandler(connection,"MessageEditionPoller"),IPeriodicProcessor(connection)
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_IDENTITYFOUND,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_NEWCHANNELMESSAGE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_NEWPRIVATEMESSAGE,this);

	Option option;
	option.Get("MessageBase",m_messagebase);

}

FreenetMessageEditionPoller::~FreenetMessageEditionPoller()
{

}

void FreenetMessageEditionPoller::FCPConnected()
{
	DateTime today;
	today.StripTime();
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblIdentity.IdentityID,tblIdentity.PublicKey,IFNULL(MAX(tblRetrievedMessageIndex.MessageIndex),0) FROM tblIdentity INNER JOIN tblRetrievedMessageIndex ON tblIdentity.IdentityID=tblRetrievedMessageIndex.IdentityID WHERE tblRetrievedMessageIndex.Date=? GROUP BY tblIdentity.IdentityID;");
	st.Bind(0,today.Format("%Y-%m-%d"));
	st.Step();
	while(st.RowReturned())
	{
		int identityid=0;
		st.ResultInt(0,identityid);
		st.ResultText(1,m_ids[identityid].m_publickey);
		st.ResultInt(2,m_lastmessageedition[today][identityid]);
		st.Step();
	}
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
	DateTime today;
	today.StripTime();
	std::map<std::string,std::string> params=flipevent.GetParameters();

	if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_IDENTITYFOUND)
	{
		DateTime date;
		int identityid=0;
		int lastedition=0;

		StringFunctions::Convert(params["lastmessageindex"],lastedition);
		StringFunctions::Convert(params["identityid"],identityid);
		DateTime::TryParse(params["date"],date);
		
		date.StripTime();

		m_lastmessageedition[date][identityid]=(std::max)(m_lastmessageedition[date][identityid],lastedition);

		if(today==date && m_lastmessageedition[date][identityid]>=0)
		{
			RequestMessageEdition(date,identityid,m_lastmessageedition[date][identityid]+1,3);
			RequestMessageEdition(date,identityid,m_lastmessageedition[date][identityid]+10);
			RequestMessageEdition(date,identityid,m_lastmessageedition[date][identityid]+25);

			m_log->Debug("FreenetMessageEditionPoller::HandleFLIPEvent started looking for new message editions for "+params["identityid"]);
		}
		else
		{
			m_log->Error("FreenetMessageEditionPoller::HandleFLIPEvent error with date or last edition in identity found handler");
		}

	}
	// we retrieved a new message - poll once for the next message edition
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_NEWCHANNELMESSAGE || flipevent.GetType()==FLIPEvent::EVENT_FREENET_NEWPRIVATEMESSAGE)
	{
		DateTime date;
		int edition=0;
		int identityid=0;

		StringFunctions::Convert(params["identityid"],identityid);
		StringFunctions::Convert(params["edition"],edition);
		DateTime::TryParse(params["sentdate"],date);

		date.StripTime();

		m_lastmessageedition[date][identityid]=(std::max)(m_lastmessageedition[date][identityid],edition);

		if(today==date && m_lastmessageedition[date][identityid]>=0)
		{
			if(edition>=m_lastmessageedition[date][identityid])
			{
				RequestMessageEdition(date,identityid,m_lastmessageedition[date][identityid]+1,2);
			}
		}
		else
		{
			m_log->Error("FreenetMessageEditionPoller::HandleFLIPEvent error with date or last edition in new message handler");
		}

	}
	return false;
}

void FreenetMessageEditionPoller::Process()
{
	std::vector<DateTime> deletedates;
	DateTime today;
	DateTime threeminutesago;

	today.SetNowUTC();
	today.StripTime();

	threeminutesago.SetNowUTC();
	threeminutesago.Add(0,-3,0,0,0,0);

	for(std::map<DateTime,std::map<int,int> >::iterator i=m_lastmessageedition.begin(); i!=m_lastmessageedition.end(); i++)
	{
		if((*i).first<today)
		{
			deletedates.push_back((*i).first);
		}
		else
		{
			for(std::map<int,int>::iterator j=(*i).second.begin(); j!=(*i).second.end(); j++)
			{
				if(m_ids[(*j).first].m_lastactivity<threeminutesago)
				{
					m_log->Debug("FreenetMessageEditionPoller::Process three minutes have passed without activity from identity.  Requesting next message edition.");
					RequestMessageEdition(today,(*j).first,(*j).second+1);
					RequestMessageEdition(today,(*j).first,(*j).second+5);
				}
			}
		}
	}

	for(std::vector<DateTime>::iterator i=deletedates.begin(); i!=deletedates.end(); i++)
	{
		m_lastmessageedition.erase((*i));
	}
}

void FreenetMessageEditionPoller::RequestMessageEdition(const DateTime &date, const int identityid, const int edition, const int priority)
{
	std::string editionstr("0");
	std::string identityidstr("");
	std::string prioritystr("");

	if(m_ids.find(identityid)==m_ids.end() || m_ids[identityid].m_publickey=="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,m_ids[identityid].m_publickey);
		}
	}

	StringFunctions::Convert(edition,editionstr);
	StringFunctions::Convert(identityid,identityidstr);
	StringFunctions::Convert(priority,prioritystr);

	if(m_ids[identityid].m_publickey!="")
	{
		FCPv2::Message mess("ClientGet");
		mess["URI"]="SSK@"+m_ids[identityid].m_publickey.substr(4)+m_messagebase+"|"+date.Format("%Y-%m-%d")+"|Message-"+editionstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+identityidstr+"|"+date.Format("%Y-%m-%d")+"|"+editionstr;
		mess["MaxSize"]="1024";
		mess["ReturnType"]="none";
		if(priority!=-1)
		{
			mess["PriorityClass"]=prioritystr;
		}

		m_fcp->Send(mess);
		m_log->Debug("FreenetMessageEditionPoller::RequestMessageEdition polling for message for "+mess["Identifier"]);
	}

	m_ids[identityid].m_lastactivity.SetNowUTC();
}
