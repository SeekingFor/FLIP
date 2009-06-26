#include "freenetunkeyedidentitycreator.h"
#include "../stringfunctions.h"
#include "../datetime.h"

FreenetUnkeyedIdentityCreator::FreenetUnkeyedIdentityCreator(FreenetConnection *connection,FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"UnkeyedIdentityCreator"),m_lastchecked(0),m_waiting(false)
{

}

FreenetUnkeyedIdentityCreator::~FreenetUnkeyedIdentityCreator()
{

}

void FreenetUnkeyedIdentityCreator::FCPConnected()
{

}

void FreenetUnkeyedIdentityCreator::FCPDisconnected()
{

}

const bool FreenetUnkeyedIdentityCreator::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		if(message.GetName()=="SSKKeypair")
		{
			DateTime now;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);

			if(idparts.size()>1)
			{
				int localidentityid=0;
				StringFunctions::Convert(idparts[1],localidentityid);

				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblLocalIdentity SET PublicKey=?, PrivateKey=? WHERE LocalIdentityID=?;");
				st.Bind(0,message["RequestURI"]);
				st.Bind(1,message["InsertURI"]);
				st.Bind(2,localidentityid);
				st.Step();

				st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES(?,?,'Local identity');");
				st.Bind(0,message["RequestURI"]);
				st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
				st.Step();

			}
			m_waiting=false;
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

void FreenetUnkeyedIdentityCreator::Process()
{
	DateTime oneminutepassed;
	DateTime tenminutespassed;
	oneminutepassed.Add(0,-1);
	tenminutespassed.Add(0,-10);
	// we aren't waiting for a response and 1 minute has passed
	if(m_waiting==false && m_lastchecked<oneminutepassed)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE PublicKey IS NULL OR PrivateKey IS NULL OR PublicKey='' OR PrivateKey='';");
		st.Step();
		if(st.RowReturned())
		{
			int localidentityid=0;
			st.ResultInt(0,localidentityid);

			StartSSKRequest(localidentityid);

			m_waiting=true;

		}
		m_lastchecked.SetNowUTC();
	}
	// we are waiting for a response, but haven't received one in 10 minutes
	else if(m_waiting==true && m_lastchecked<tenminutespassed)
	{
		m_waiting=false;
		m_lastchecked.SetNowUTC();
		m_log->Error("FreenetUnkeyedIdentityCreator::Process more than 10 minutes have passed without a node response");
	}
}

void FreenetUnkeyedIdentityCreator::StartSSKRequest(const int localidentityid)
{
	std::string localidentityidstr("");
	StringFunctions::Convert(localidentityid,localidentityidstr);
	FCPv2::Message mess("GenerateSSK");

	mess["Identifier"]=m_fcpuniqueidentifier+"|"+localidentityidstr;
	m_fcp->Send(mess);
}
