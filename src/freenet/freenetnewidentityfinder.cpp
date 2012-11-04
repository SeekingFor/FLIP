#include "freenetnewidentityfinder.h"
#include "../option.h"
#include "../stringfunctions.h"

FreenetNewIdentityFinder::FreenetNewIdentityFinder(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"NewIdentityFinder"),m_waiting(false)
{
	Option option;
	option.Get("MessageBase",m_messagebase);
}

FreenetNewIdentityFinder::~FreenetNewIdentityFinder()
{

}

void FreenetNewIdentityFinder::FCPConnected()
{
	m_lastactivity.SetNowUTC();
	m_waiting=false;
}

void FreenetNewIdentityFinder::FCPDisconnected()
{
	m_waiting=false;
}

const bool FreenetNewIdentityFinder::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{

		m_lastactivity.SetNowUTC();

		if(message.GetName()=="AllData")
		{
			DateTime now;
			std::vector<std::string> idparts;
			int datalength=0;

			StringFunctions::Convert(message["DataLength"],datalength);
			StringFunctions::Split(message["Identifier"],"|",idparts);

			if(idparts.size()>2)
			{
				SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblAnnounceIndex(Date,AnnounceIndex,Done) VALUES(?,?,1);");
				st.Bind(0,idparts[2]);
				st.Bind(1,idparts[1]);
				st.Step();
			}

			if(m_fcp->WaitForBytes(10,datalength))
			{
				std::string publickey("");
				std::vector<char> data;
				m_fcp->Receive(data,datalength);

				publickey.insert(publickey.end(),data.begin(),data.end());
				if(publickey!="" && publickey.find("SSK@")==0 && publickey.find(",")!=std::string::npos && publickey.find("/")==publickey.size()-1)
				{
					SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES(?,?,'Automatic announcement');");
					st.Bind(0,publickey);
					st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
					st.Step();
				}
				m_log->Debug("FreenetNewIdentityFinder::HandleFCPMessage received AllData for "+message["Identifier"]);
			}
			else
			{
				m_log->Error("FreenetNewIdentityFinder::HandleFCPMessage couldn't receive AllData for "+message["Identifier"]);
			}
			m_waiting=false;
			return true;
		}
		else if(message.GetName()=="DataFound")
		{
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			return true;
		}
		else if(message.GetName()=="GetFailed")
		{
			if(message["Fatal"]=="true")
			{
				std::vector<std::string> idparts;
				StringFunctions::Split(message["Identifier"],"|",idparts);

				if(idparts.size()>2)
				{
					SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblAnnounceIndex(Date,AnnounceIndex,Done) VALUES(?,?,1);");
					st.Bind(0,idparts[2]);
					st.Bind(1,idparts[1]);
					st.Step();
				}
			}
			m_waiting=false;
			return true;
		}
	}
	return false;
}

void FreenetNewIdentityFinder::Process()
{
	DateTime tenminutesago;
	tenminutesago.Add(0,-10);
	if(m_waiting==false || m_lastactivity<tenminutesago)
	{
		StartRequest();
	}
}

void FreenetNewIdentityFinder::StartRequest()
{
	DateTime now;
	SQLite3DB::Statement st=m_db->Prepare("SELECT IFNULL(MAX(AnnounceIndex)+1,0) FROM tblAnnounceIndex WHERE Date=?;");
	st.Bind(0,now.Format("%Y-%m-%d"));
	st.Step();

	if(st.RowReturned())
	{
		std::string indexstr("0");
		st.ResultText(0,indexstr);

		FCPv2::Message mess("ClientGet");
		mess["URI"]="KSK@"+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Announce|"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+indexstr+"|"+now.Format("%Y-%m-%d")+"|"+mess["URI"];
		mess["ReturnType"]="direct";
		mess["MaxSize"]="512";

		m_fcp->Send(mess);

		m_log->Debug("FreenetNewIdentityFinder::StartRequest requesting "+mess["URI"]);

	}

	m_lastactivity.SetNowUTC();
	m_waiting=true;

}
