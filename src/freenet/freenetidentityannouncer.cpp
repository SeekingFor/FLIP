#include "freenetidentityannouncer.h"
#include "../flipeventsource.h"
#include "../stringfunctions.h"
#include "../option.h"

FreenetIdentityAnnouncer::FreenetIdentityAnnouncer(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"IdentityAnnouncer"),m_lastactivity(0)
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERREGISTER,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERQUIT,this);

	Option option;
	option.Get("MessageBase",m_messagebase);

	m_db->Execute("DELETE FROM tblAnnounceIndex WHERE Done=0;");

}

FreenetIdentityAnnouncer::~FreenetIdentityAnnouncer()
{

}

void FreenetIdentityAnnouncer::FCPConnected()
{
	m_lastactivity.SetNowUTC();
	for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
	{
		(*i).second.m_announcing=false;
	}
	m_db->Execute("DELETE FROM tblAnnounceIndex WHERE Done=0;");
}

void FreenetIdentityAnnouncer::FCPDisconnected()
{
	m_lastactivity.SetNowUTC();
	m_db->Execute("DELETE FROM tblAnnounceIndex WHERE Done=0;");
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
			m_ids[id].m_lastannounced.SetNowUTC();
			m_ids[id].m_announcing=false;

			SQLite3DB::Statement st=m_db->Prepare("INSERT OR REPLACE INTO tblAnnounceIndex(Date,AnnounceIndex,Done) VALUES(?,?,1);");
			st.Bind(0,idparts[2]);
			st.Bind(1,index);
			st.Step();

			m_log->Debug("FreenetIdentityAnnouncer::HandleFCPMessage announced "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="PutFailed")
		{
			m_log->Debug("FreenetIdentityAnnouncer::HandleFCPMessage PutFailed "+message["Identifier"]+"  Code="+message["Code"]+"  Description="+message["CodeDescription"]);
			m_ids[id].m_announcing=false;

			if(message["Fatal"]=="true")
			{
				SQLite3DB::Statement st=m_db->Prepare("INSERT OR REPLACE INTO tblAnnounceIndex(Date,AnnounceIndex,Done) VALUES(?,?,1);");
				st.Bind(0,idparts[2]);
				st.Bind(1,index);
				st.Step();
			}

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
		m_ids[id].m_lastannounced.Add(0,0,0,-1);
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
		DateTime now;
		DateTime tenminutespassed;
		DateTime sixhourspassed;

		tenminutespassed.Add(0,-10);
		sixhourspassed.Add(0,0,-6);

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
			if((*i).second.m_active && (*i).second.m_announcing==false && ((*i).second.m_lastannounced<sixhourspassed || (*i).second.m_lastannounced.Day()!=now.Day()))
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
		mess["ExtraInsertsSingleBlock"]="0";
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datalengthstr;

		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_ids[localidentityid].m_announcing=true;

		st=m_db->Prepare("INSERT INTO tblAnnounceIndex(Date,AnnounceIndex) VALUES(?,?);");
		st.Bind(0,now.Format("%Y-%m-%d"));
		st.Bind(1,indexstr);
		st.Step();

		m_log->Debug("FreenetIdentityAnnouncer::StartInsert started announce "+mess["Identifier"]);

	}

	m_lastactivity.SetNowUTC();

}
