#include "freenetmessagefinder.h"
#include "../option.h"
#include "../stringfunctions.h"

#include <vector>

FreenetMessageFinder::FreenetMessageFinder(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"MessageFinder")
{

	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_IDENTITYFOUND,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IDENTITYACTIVE,this);

	Option option;
	option.Get("MessageBase",m_messagebase);

}

FreenetMessageFinder::~FreenetMessageFinder()
{

}

void FreenetMessageFinder::FCPConnected()
{

	m_ids.clear();
	m_subscribed.clear();

	StartRequests();
}

void FreenetMessageFinder::FCPDisconnected()
{

}

const bool FreenetMessageFinder::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		if(message.GetName()=="SubscribedUSK")
		{
			return true;
		}
		else if(message.GetName()=="SubscribedUSKUpdate")
		{
			int identityid=0;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);

			// dispatch new message edition found event
			std::map<std::string,std::string> params;
			if(idparts.size()>2)
			{
				StringFunctions::Convert(idparts[1],identityid);
				params["identityid"]=idparts[1];
				params["date"]=idparts[2];
			}
			params["edition"]=message["Edition"];
			DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_FREENET_NEWMESSAGEEDITION,params));

			m_ids[identityid].m_lastactive.SetNowUTC();

			m_log->Debug("FreenetMessageFinder::HandleFCPMessage received SubscribedUSKUpdate for "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			return true;
		}
	}
	return false;
}

const bool FreenetMessageFinder::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	if(flipevent.GetType()==flipevent.EVENT_FREENET_IDENTITYFOUND)
	{
		DateTime date;
		DateTime now;
		std::map<std::string,std::string> params=flipevent.GetParameters();
		int id=0;
		int lastindex=-1;

		StringFunctions::Convert(params["identityid"],id);
		StringFunctions::Convert(params["lastmessageindex"],lastindex);

		std::vector<std::string> dateparts;
		StringFunctions::Split(params["date"],"-",dateparts);
		if(dateparts.size()==3)
		{
			int year=0;
			int month=0;
			int day=0;
			StringFunctions::Convert(dateparts[0],year);
			StringFunctions::Convert(dateparts[1],month);
			StringFunctions::Convert(dateparts[2],day);
			date.Set(year,month,day,0,0,0);
		}

		if(id!=0 && lastindex>-1)
		{
			std::string publickey("");

			SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
			st.Bind(0,id);
			st.Step();
			if(st.RowReturned())
			{
				st.ResultText(0,publickey);
			}

			if(m_subscribed[date].find(id)==m_subscribed[date].end())
			{
				Subscribe(id,publickey,date,lastindex+1);
			}

			m_ids[id].m_lastactive.SetNowUTC();

		}
		else
		{
			m_log->Debug("FreenetMessageFinder::HandleFLIPEvent bad parameters");
		}
		return true;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_IDENTITYACTIVE)
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		int id=0;

		StringFunctions::Convert(params["identityid"],id);

		m_ids[id].m_lastactive.SetNowUTC();

		return true;
	}

	return false;
}

void FreenetMessageFinder::Process()
{
	DateTime now;
	DateTime currentday;
	DateTime fiveminutes;
	DateTime fiveminutesday;
	DateTime thirtyminutesago;

	fiveminutes.Add(0,5);
	fiveminutesday.Add(0,5);
	fiveminutesday.StripTime();
	thirtyminutesago.Add(0,-30);

	currentday.StripTime();
	std::vector<DateTime> m_erase;
	std::vector<std::pair<int,DateTime> > m_unsubscribe;

	// check if or we are close to changing date and subscribe to all existing connections on new date
	if(now.Day()!=fiveminutes.Day())
	{
		for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
		{
			if(m_ids[(*i).first].m_lastactive>=thirtyminutesago && m_subscribed[fiveminutesday].find((*i).first)==m_subscribed[fiveminutesday].end())
			{
				std::string publickey("");
				SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
				st.Bind(0,(*i).first);
				st.Step();
				if(st.RowReturned())
				{
					st.ResultText(0,publickey);
				}

				Subscribe((*i).first,publickey,fiveminutesday,0);
			}
		}
	}

	// check if date has changed and unsubscribe previous subscriptions and make sure current days subscriptions are active
	for(std::map<DateTime,std::set<int> >::iterator i=m_subscribed.begin(); i!=m_subscribed.end(); )
	{
		if((*i).first<currentday)
		{
			for(std::set<int>::iterator j=(*i).second.begin(); j!=(*i).second.end(); j++)
			{
				if(m_ids[(*j)].m_lastactive>=thirtyminutesago && m_subscribed[currentday].find((*j))==m_subscribed[currentday].end())
				{
					std::string publickey("");
					SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
					st.Bind(0,(*j));
					st.Step();
					if(st.RowReturned())
					{
						st.ResultText(0,publickey);
					}

					Subscribe((*j),publickey,currentday,0);
				}
				//Unsubscribe((*j),(*i).first);
				m_unsubscribe.push_back(std::pair<int,DateTime>((*j),(*i).first));
			}
			//i=m_subscribed.erase(i);	// Linux doesn't like this so store dates to delete in a vector and delete them afterwards
			m_erase.push_back((*i).first);
			i++;
		}
		else
		{
			i++;
		}
	}

	for(std::vector<std::pair<int,DateTime> >::iterator i=m_unsubscribe.begin(); i!=m_unsubscribe.end(); i++)
	{
		Unsubscribe((*i).first,(*i).second);
	}

	for(std::vector<DateTime>::iterator i=m_erase.begin(); i!=m_erase.end(); i++)
	{
		m_subscribed.erase((*i));
	}

	// check if an identity hasn't been active in 30 minutes and unsubscribe and send inactive event
	for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
	{
		if((*i).second.m_lastactive<thirtyminutesago)
		{
			for(std::map<DateTime,std::set<int> >::iterator j=m_subscribed.begin(); j!=m_subscribed.end(); j++)
			{
				std::set<int>::iterator pos;
				if((pos=(*j).second.find((*i).first))!=(*j).second.end())
				{
					Unsubscribe((*i).first,(*j).first);

					std::map<std::string,std::string> params;
					StringFunctions::Convert((*i).first,params["identityid"]);
					params["sentdate"]=now.Format("%Y-%m-%d %H:%M:%S");
					DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_FREENET_IDENTITYINACTIVE,params));
				}
			}
		}
	}
}

void FreenetMessageFinder::StartRequests()
{
	DateTime today;
	DateTime thirtyminutesago;

	thirtyminutesago.Add(0,-30,0,0);

	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID, PublicKey, IFNULL((SELECT MAX(MessageIndex)+1 FROM tblRetrievedMessageIndex WHERE tblRetrievedMessageIndex.IdentityID=tblIdentity.IdentityID AND tblRetrievedMessageIndex.Date=?),0) FROM tblIdentity WHERE LastSeen>=?;");
	st.Bind(0,today.Format("%Y-%m-%d"));
	st.Bind(1,thirtyminutesago.Format("%Y-%m-%d"));
	st.Step();

	while(st.RowReturned())
	{

		int id=0;
		std::string publickey("");
		int edition=-1;

		st.ResultInt(0,id);
		st.ResultText(1,publickey);
		st.ResultInt(2,edition);

		m_ids[id];
		Subscribe(id,publickey,today,edition);

		st.Step();
	}

}

void FreenetMessageFinder::Subscribe(const int identityid, const std::string &publickey, const DateTime date, const int edition)
{
	DateTime day;
	std::string idstr("");
	std::string editionstr("0");
	FCPv2::Message mess("SubscribeUSK");

	StringFunctions::Convert(edition,editionstr);
	StringFunctions::Convert(identityid,idstr);

	mess["URI"]="USK@"+publickey.substr(4)+m_messagebase+"|"+date.Format("%Y-%m-%d")+"|Message/"+editionstr;
	mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+date.Format("%Y-%m-%d");
	mess["PriorityClass"]="3";
	mess["PriorityClassProgress"]="2";

	m_fcp->Send(mess);

	day.Set(date.Year(),date.Month(),date.Day(),0,0,0);
	m_subscribed[day].insert(identityid);

	m_log->Debug("FreenetMessageFinder::Subscribe subscribed to "+mess["URI"]);
}

void FreenetMessageFinder::Unsubscribe(const int identityid, const DateTime date)
{
	DateTime day;
	std::string idstr("");
	FCPv2::Message mess("UnsubscribeUSK");

	StringFunctions::Convert(identityid,idstr);

	mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+date.Format("%Y-%m-%d");
	m_fcp->Send(mess);

	day.Set(date.Year(),date.Month(),date.Day(),0,0,0);
	m_subscribed[day].erase(identityid);

	m_log->Debug("FreenetMessageFinder::Unsubscribe unsubscribed "+mess["Identifier"]);
}
