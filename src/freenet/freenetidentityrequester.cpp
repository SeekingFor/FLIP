#include "freenetidentityrequester.h"
#include "freenetmessage.h"
#include "../option.h"
#include "../stringfunctions.h"
#include "../irc/ircnick.h"

#include <algorithm>

FreenetIdentityRequester::FreenetIdentityRequester(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"IdentityRequester"),m_maxrequests(5)
{
	m_lastloadedids.Add(0,0,0,0,0,-1);

	Option option;
	option.Get("MessageBase",m_messagebase);

}

FreenetIdentityRequester::~FreenetIdentityRequester()
{

}

void FreenetIdentityRequester::FCPConnected()
{
	m_lastactivity.SetNowUTC();
	m_ids.clear();
}

void FreenetIdentityRequester::FCPDisconnected()
{

}

const bool FreenetIdentityRequester::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		DateTime now;
		std::vector<std::string> idparts;
		int identityid=0;

		m_lastactivity.SetNowUTC();

		StringFunctions::Split(message["Identifier"],"|",idparts);
		StringFunctions::Convert(idparts[1],identityid);

		if(message.GetName()=="AllData")
		{
			FreenetMessage fm;
			std::vector<char> data;
			int datalength=0;

			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblIdentityEdition(IdentityID,Date,Edition,Found) VALUES(?,?,?,1);");
			st.Bind(0,identityid);
			st.Bind(1,idparts[2]);
			st.Bind(2,idparts[3]);
			st.Step();

			StringFunctions::Convert(message["DataLength"],datalength);

			if(m_fcp->WaitForBytes(50,datalength))
			{
				
				m_fcp->Receive(data,datalength);

				if(FreenetMessage::TryParse(std::string(data.begin(),data.end()),fm))
				{
					if(IRCNick::IsValid(fm["name"]))
					{
						st=m_db->Prepare("UPDATE tblIdentity SET Name=?, LastSeen=?, RSAPublicKey=? WHERE IdentityID=?;");
						st.Bind(0,fm["name"]);
						st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
						st.Bind(2,fm["rsapublickey"]);
						st.Bind(3,identityid);
						st.Step();

						std::map<std::string,std::string> eventparams;
						eventparams["name"]=fm["name"];
						eventparams["identityid"]=idparts[1];
						eventparams["date"]=idparts[2];
						eventparams["lastmessageindex"]=fm["lastmessageindex"];

						// dispatch event for finding active id
						DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_FREENET_IDENTITYFOUND,eventparams));

						m_log->Debug("FreenetIdentityRequester::HandleFCPMessage retrieved identity "+message["Identifier"]);

					}
					else
					{
						m_log->Error("FreenetIdentityRequester::HandleFCPMessage nick name invalid in "+message["Identifier"]);
					}
				}
				else
				{
					m_log->Error("FreenetIdentityRequester::HandleFCPMessage couldn't parse message "+message["Identifier"]);
				}

			}

			RemoveRequest(identityid);
			return true;
		}
		else if(message.GetName()=="GetFailed")
		{

			if(message["Fatal"]=="true")
			{
				SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblIdentityEdition(IdentityID,Date,Edition,Found) VALUES(?,?,?,0);");
				st.Bind(0,identityid);
				st.Bind(1,idparts[2]);
				st.Bind(2,idparts[3]);
				st.Step();
			}

			if(message["Code"]=="27")
			{
				std::vector<std::string> uriparts;
				std::string newuri=StringFunctions::UriDecode(message["RedirectURI"]);
				if(newuri.size()>0 && newuri[newuri.size()-1]=='/')
				{
					newuri.erase(newuri.size()-1);
				}
				StringFunctions::Split(newuri,"/",uriparts);
				if(uriparts.size()>1)
				{
					int edition=0;
					StringFunctions::Convert(uriparts[uriparts.size()-1],edition);

					StartRequest(identityid,edition);

					m_log->Debug("FreenetIdentityRequester::HandleFCPMessage starting redirect for "+message["Identifier"]);
				}
			}

			m_log->Debug("FreenetIdentityRequester::HandleFCPMessage GetFailed for "+message["Identifier"]);
			RemoveRequest(identityid);
			return true;
		}
		else if(message.GetName()=="DataFound")
		{
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			m_log->Debug("FreenetIdentityRequester::HandleFCPMessage IdentifierCollision for "+message["Identifier"]);
			RemoveRequest(identityid);
			return true;
		}
	}
	return false;
}

void FreenetIdentityRequester::Process()
{
	DateTime now;
	DateTime oneminuteago;
	DateTime tenminutesago;

	oneminuteago.Add(0,-1);
	tenminutesago.Add(0,-10);

	if(m_ids.size()==0 && m_lastloadedids<oneminuteago)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE Ignored=0;");
		st.Step();
		while(st.RowReturned())
		{
			int id=0;
			st.ResultInt(0,id);
			m_ids.insert(id);
			st.Step();
		}
		m_lastloadedids.SetNowUTC();
	}

	if(m_ids.size()>0 && (m_requesting.size()<m_maxrequests || m_lastactivity<tenminutesago))
	{
		StartRequest((*m_ids.begin()));
	}

}

void FreenetIdentityRequester::RemoveRequest(const int identityid)
{
	std::set<int>::iterator pos=std::find(m_requesting.begin(),m_requesting.end(),identityid);
	if(pos!=m_requesting.end())
	{
		m_requesting.erase(pos);
	}
}

void FreenetIdentityRequester::StartRequest(const int identityid, const int edition)
{
	DateTime now;
	FCPv2::Message mess("ClientGet");
	std::string publickey("");
	std::string idstr("");
	std::string editionstr("");

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey, IFNULL((SELECT MAX(Edition)+1 FROM tblIdentityEdition WHERE IdentityID=? AND Date=?),0) FROM tblIdentity WHERE tblIdentity.IdentityID=?;");
	st.Bind(0,identityid);
	st.Bind(1,now.Format("%Y-%m-%d"));
	st.Bind(2,identityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);
		if(edition==-1)
		{
			st.ResultText(1,editionstr);
		}
		else
		{
			StringFunctions::Convert(edition,editionstr);
		}

		StringFunctions::Convert(identityid,idstr);

		mess["URI"]="USK@"+publickey.substr(4)+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Identity/"+editionstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+now.Format("%Y-%m-%d")+"|"+editionstr+"|"+mess["URI"];
		mess["ReturnType"]="direct";
		mess["MaxSize"]="1024";

		m_fcp->Send(mess);

		m_requesting.insert(identityid);

		m_log->Debug("FreenetIdentityRequester::StartRequest started request for "+mess["Identifier"]);
	}

	std::set<int>::iterator pos=std::find(m_ids.begin(),m_ids.end(),identityid);
	if(pos!=m_ids.end())
	{
		m_ids.erase(pos);
	}
	m_lastactivity.SetNowUTC();
}
