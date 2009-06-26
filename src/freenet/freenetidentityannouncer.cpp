#include "freenetidentityannouncer.h"
#include "../flipeventsource.h"
#include "../stringfunctions.h"
#include "../option.h"

FreenetIdentityAnnouncer::FreenetIdentityAnnouncer(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"IdentityAnnouncer"),m_lastactivity(0)
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERREGISTER,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERQUIT,this);
}

FreenetIdentityAnnouncer::~FreenetIdentityAnnouncer()
{

}

void FreenetIdentityAnnouncer::FCPConnected()
{
	Option option;
	option.Get("MessageBase",m_messagebase);
	m_lastactivity.SetNowUTC();
	for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
	{
		(*i).second.m_announcing=false;
	}
}

void FreenetIdentityAnnouncer::FCPDisconnected()
{
	m_lastactivity.SetNowUTC();
}

const bool FreenetIdentityAnnouncer::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		std::vector<std::string> idparts;
		StringFunctions::Split(message["Identifier"],"|",idparts);

		int index=0;
		int id=0;

		if(idparts.size()>4)
		{
			StringFunctions::Convert(idparts[1],id);
			StringFunctions::Convert(idparts[3],index);
		}

		m_lastactivity.SetNowUTC();

		if(message.GetName()=="PutSuccessful")
		{
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblAnnounceIndex(Date,AnnounceIndex) VALUES(?,?);");
			st.Bind(0,idparts[2]);
			st.Bind(1,index);
			st.Step();

			m_ids[id].m_lastannounced.SetNowUTC();
			m_ids[id].m_announcing=false;

			m_log->Debug("FreenetIdentityAnnouncer::HandleFCPMessage announced "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="PutFailed")
		{
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblAnnounceIndex(Date,AnnounceIndex) VALUES(?,?);");
			st.Bind(0,idparts[2]);
			st.Bind(1,index);
			st.Step();

			m_log->Debug("FreenetIdentityAnnouncer::HandleFCPMessage putfailed "+message["Identifier"]);
			m_ids[id].m_announcing=false;
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			return true;
		}
		else if(message.GetName()=="URIGenerated")
		{
			return true;
		}
		
	}
	return false;
}

const bool FreenetIdentityAnnouncer::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	std::map<std::string,std::string> params=flipevent.GetParameters();
	if(flipevent.GetType()==FLIPEvent::EVENT_IRC_USERREGISTER)
	{
		int id=0;
		StringFunctions::Convert(params["localidentityid"],id);
		m_ids[id].m_active=true;
		m_ids[id].m_lastannounced.Add(0,0,-1);
		m_ids[id].m_announcing=false;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_IRC_USERQUIT)
	{
		int id=0;
		StringFunctions::Convert(params["localidentityid"],id);
		m_ids[id].m_active=false;
	}
	return false;
}

void FreenetIdentityAnnouncer::Process()
{
	if(m_ids.size()>0)
	{
		DateTime tenminutespassed;
		DateTime thirtyminutespassed;

		tenminutespassed.Add(0,-10);
		thirtyminutespassed.Add(0,-30);

		if(m_lastactivity<tenminutespassed)
		{
			for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
			{
				(*i).second.m_announcing=false;
			}
			m_lastactivity.SetNowUTC();
		}

		for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
		{
			if((*i).second.m_active && ((*i).second.m_announcing==false && (*i).second.m_lastannounced<thirtyminutespassed))
			{
				StartInsert((*i).first);
			}
		}

	}
}

void FreenetIdentityAnnouncer::StartInsert(const int localidentityid)
{
	DateTime now;
	std::string publickey=m_ids[localidentityid].m_publickey;
	if(publickey=="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,localidentityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,publickey);
			m_ids[localidentityid].m_publickey=publickey;
		}
	}

	if(publickey!="")
	{
		std::string data("");
		std::string datalengthstr("");
		std::string idstr("");
		std::string indexstr("0");

		data=publickey;

		StringFunctions::Convert(localidentityid,idstr);
		StringFunctions::Convert(data.size(),datalengthstr);

		SQLite3DB::Statement st=m_db->Prepare("SELECT IFNULL(MAX(AnnounceIndex)+1,0) FROM tblAnnounceIndex WHERE Date=?;");
		st.Bind(0,now.Format("%Y-%m-%d"));
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,indexstr);
		}

		FCPv2::Message mess("ClientPut");
		mess["URI"]="KSK@"+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Announce|"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+now.Format("%Y-%m-%d")+"|"+indexstr+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datalengthstr;

		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_ids[localidentityid].m_announcing=true;

		m_log->Debug("FreenetIdentityAnnouncer::StartInsert started announce "+mess["Identifier"]);

	}

	m_lastactivity.SetNowUTC();

}
