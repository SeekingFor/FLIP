#include "freenetidentityinserter.h"
#include "freenetmessage.h"
#include "../flipeventsource.h"
#include "../option.h"
#include "../stringfunctions.h"

FreenetIdentityInserter::FreenetIdentityInserter(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"IdentityInserter"),m_inserting(false),m_lastactivity()
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERREGISTER,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_USERQUIT,this);

	Option option;
	option.Get("MessageBase",m_messagebase);

	m_timeout=10;

}

FreenetIdentityInserter::~FreenetIdentityInserter()
{

}

void FreenetIdentityInserter::FCPConnected()
{
	m_inserting=false;
	m_lastactivity.SetNowUTC();
	m_timeout=10;
}

void FreenetIdentityInserter::FCPDisconnected()
{
	m_inserting=false;
}

const bool FreenetIdentityInserter::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		int id=0;
		std::vector<std::string> idparts;
		StringFunctions::Split(message["Identifier"],"|",idparts);
		if(idparts.size()>1)
		{
			StringFunctions::Convert(idparts[1],id);
		}

		m_inserting=false;
		m_lastactivity.SetNowUTC();

		if(message.GetName()=="PutSuccessful")
		{
			m_activeids[id].m_lastinserted.SetNowUTC();
			m_log->Debug("FreenetIdentityInserter::HandleFCPMessage PutSuccessful for "+message["Identifier"]);
			m_timeout=10;
			return true;
		}
		else if(message.GetName()=="PutFailed")
		{
			m_log->Error("FreenetIdentityInserter::HandleFCPMessage PutFailed for "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			m_log->Error("FreenetIdentityInserter::HandleFCPMessage IdentifierCollision for "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="URIGenerated")
		{
			return true;
		}

		return false;
	}
	return false;
}

const bool FreenetIdentityInserter::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	if(flipevent.GetType()==FLIPEvent::EVENT_IRC_USERREGISTER && flipevent.GetParameters().size()>0)
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, PrivateKey, Name FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,params["localidentityid"]);
		st.Step();
		if(st.RowReturned())
		{
			int id=0;
			std::string privatekey("");
			std::string name("");

			st.ResultInt(0,id);
			st.ResultText(1,privatekey);
			st.ResultText(2,name);

			if(m_activeids.find(id)==m_activeids.end())
			{
				// set 1 year back so we insert this one asap
				m_activeids[id].m_lastinserted.Add(0,0,0,0,0,-1);
				m_activeids[id].m_privatekey=privatekey;
				m_activeids[id].m_name=name;
				m_lastactivity.SetNowUTC();
			}
		}
		return true;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_IRC_USERQUIT)
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,params["localidentityid"]);
		st.Step();
		if(st.RowReturned())
		{
			int id=0;
			st.ResultInt(0,id);
			m_activeids.erase(id);
		}
		return true;
	}
	return false;
}

void FreenetIdentityInserter::Process()
{
	DateTime now;
	DateTime timeoutpassed;
	timeoutpassed.Add(0,-m_timeout);
	if(m_activeids.size()>0 && (m_inserting==false || m_lastactivity<timeoutpassed))
	{
		if(m_lastactivity<timeoutpassed)
		{
			std::string timeoutstr("");
			StringFunctions::Convert(m_timeout,timeoutstr);
			m_log->Error("FreenetIdentityInserter::Process more than "+timeoutstr+" minutes have passed without a response from the node");
			if(m_timeout<60)
			{
				m_timeout+=10;
			}
		}

		int id=(*m_activeids.begin()).first;
		DateTime lastdate=(*m_activeids.begin()).second.m_lastinserted;
		for(std::map<int,idinfo>::iterator i=m_activeids.begin(); i!=m_activeids.end(); i++)
		{
			if((*i).second.m_lastinserted<lastdate)
			{
				id=(*i).first;
				lastdate=(*i).second.m_lastinserted;
			}
		}

		// only insert id every x minutes, or if date has changed
		if(lastdate<timeoutpassed || lastdate.Day()!=now.Day())
		{
			StartIDInsert(id);
		}

		m_lastactivity.SetNowUTC();
	}
}

void FreenetIdentityInserter::StartIDInsert(const int id)
{
	std::string privatekey=m_activeids[id].m_privatekey;
	if(privatekey=="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,id);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,privatekey);
			m_activeids[id].m_privatekey=privatekey;
		}
	}

	if(privatekey!="")
	{
		DateTime now;
		std::string data("");
		std::string idstr("");
		std::string datalengthstr("");
		std::string lastindexstr("0");
		std::string rsapublickey("");

		SQLite3DB::Statement st=m_db->Prepare("SELECT IFNULL(MAX(MessageIndex),0) FROM tblInsertedMessageIndex WHERE LocalIdentityID=? AND Date=? AND Inserted=1;");
		st.Bind(0,id);
		st.Bind(1,now.Format("%Y-%m-%d"));
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,lastindexstr);
		}

		st=m_db->Prepare("SELECT RSAPublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,id);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,rsapublickey);
		}

		FreenetMessage fm;
		fm["name"]=m_activeids[id].m_name;
		fm["lastmessageindex"]=lastindexstr;
		fm["rsapublickey"]=rsapublickey;
		data=fm.GetMessageText();

		StringFunctions::Convert(id,idstr);
		StringFunctions::Convert(data.size(),datalengthstr);

		FCPv2::Message mess("ClientPut");
		mess["URI"]="USK@"+privatekey.substr(4)+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Identity/0";
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+now.Format("%Y-%m-%d")+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datalengthstr;
		mess["Metadata.ContentType"]="";

		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_log->Debug("FreenetIdentityInserter::StartIDInsert started insert of "+m_activeids[id].m_name+" at "+mess["URI"]);

		m_inserting=true;

	}

	m_activeids[id].m_lastinserted.SetNowUTC();

}
