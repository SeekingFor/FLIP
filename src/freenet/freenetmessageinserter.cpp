#include "freenetmessageinserter.h"
#include "freenetmessage.h"
#include "../flipeventsource.h"
#include "../option.h"
#include "../datetime.h"
#include "../stringfunctions.h"
#include "../rsakeypair.h"

FreenetMessageInserter::FreenetMessageInserter(FreenetConnection *connection, FCPv2::Connection *fcp):IFCPConnected(fcp,connection),IPeriodicProcessor(connection),IFCPMessageHandler(connection,"MessageInserter")
{
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_CHANNELMESSAGE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_IRC_PRIVATEMESSAGE,this);

	Option option;
	option.Get("MessageBase",m_messagebase);

}

FreenetMessageInserter::~FreenetMessageInserter()
{

}

void FreenetMessageInserter::FCPConnected()
{

}

void FreenetMessageInserter::FCPDisconnected()
{

}

const int FreenetMessageInserter::GetNextMessageIndex(const int localidentityid, const DateTime &date)
{
	int index=0;
	SQLite3DB::Statement st=m_db->Prepare("SELECT IFNULL(MAX(MessageIndex)+1,0) FROM tblInsertedMessageIndex WHERE LocalIdentityID=? AND Date=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,date.Format("%Y-%m-%d"));
	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,index);
	}
	return index;
}

const bool FreenetMessageInserter::HandleFCPMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find(m_fcpuniqueidentifier)==0)
	{
		
		if(message.GetName()=="PutSuccessful")
		{
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			
			if(idparts.size()>4)
			{
				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblInsertedMessageIndex SET Inserted=1 WHERE LocalIdentityID=? AND Date=? AND MessageIndex=?;");
				st.Bind(0,idparts[1]);
				st.Bind(1,idparts[2]);
				st.Bind(2,idparts[3]);
				st.Step();
			}

			m_log->Debug("FreenetMessageInserter::HandleFCPMessage PutSuccessful for "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="PutFailed")
		{
			// TODO - try insert again
			m_log->Error("FreenetMessageInserter::HandleFCPMessage PutFailed for "+message["Identifier"]);
			return true;
		}
		else if(message.GetName()=="IdentifierCollision")
		{
			m_log->Error("FreenetMessageInserter::HandleFCPMessage IdentifierCollision for "+message["Identifier"]);
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

const bool FreenetMessageInserter::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	if(flipevent.GetType()==FLIPEvent::EVENT_IRC_CHANNELMESSAGE && m_fcp->IsConnected())
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		int id=0;
		StringFunctions::Convert(params["localidentityid"],id);

		StartChannelInsert(id,params["channel"],params["message"]);

		return true;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_IRC_PRIVATEMESSAGE && m_fcp->IsConnected())
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		int localidentityid=0;
		int recipientidentityid=0;

		StringFunctions::Convert(params["localidentityid"],localidentityid);
		StringFunctions::Convert(params["recipientidentityid"],recipientidentityid);

		StartPrivateInsert(localidentityid,recipientidentityid,params["message"]);

		return true;

	}
	return false;
}

void FreenetMessageInserter::LoadLocalIdentityPrivateKey(const int localidentityid)
{
	if(m_identitykeys[localidentityid]=="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,localidentityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,m_identitykeys[localidentityid]);
		}
	}
}

void FreenetMessageInserter::Process()
{

}

void FreenetMessageInserter::StartChannelInsert(const int localidentityid, const std::string &channel, const std::string &message)
{
	SQLite3DB::Statement st;
	std::string idstr("");
	std::string data("");
	std::string datalengthstr("");
	DateTime now;
	FCPv2::Message mess("ClientPut");

	LoadLocalIdentityPrivateKey(localidentityid);

	if(m_identitykeys[localidentityid]!="")
	{
		std::string indexstr("0");
		FreenetMessage fm;

		fm["type"]="channelmessage";
		fm["channel"]=channel;
		fm["sentdate"]=now.Format("%Y-%m-%d %H:%M:%S");
		fm.Body()=message;
		data=fm.GetMessageText();

		StringFunctions::Convert(localidentityid,idstr);
		StringFunctions::Convert(data.size(),datalengthstr);
		StringFunctions::Convert(GetNextMessageIndex(localidentityid,now),indexstr);

		st=m_db->Prepare("INSERT INTO tblInsertedMessageIndex(LocalIdentityID,Date,MessageIndex) VALUES(?,?,?);");
		st.Bind(0,localidentityid);
		st.Bind(1,now.Format("%Y-%m-%d"));
		st.Bind(2,indexstr);
		st.Step();

		mess["URI"]="SSK@"+m_identitykeys[localidentityid].substr(4)+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Message-"+indexstr;
		mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+now.Format("%Y-%m-%d")+"|"+indexstr+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datalengthstr;

		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_log->Debug("FreenetMessageInserter::StartInsert started insert of channel message "+mess["Identifier"]);
	}
}

void FreenetMessageInserter::StartPrivateInsert(const int localidentityid, const int recipientidentityid, const std::string &message)
{
	RSAKeyPair rsa;
	std::string idstr("");
	std::string publickey("");
	std::string rsapublickey("");
	std::string data("");
	std::string datalengthstr("");
	DateTime now;
	FCPv2::Message mess("ClientPut");

	LoadLocalIdentityPrivateKey(localidentityid);

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey, RSAPublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,recipientidentityid);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,publickey);
		st.ResultText(1,rsapublickey);
	}

	if(publickey!="" && rsapublickey!="" && rsa.SetFromEncodedPublicKey(rsapublickey))
	{
		if(m_identitykeys[localidentityid]!="")
		{
			std::string indexstr("0");
			FreenetMessage fm;

			fm["type"]="privatemessage";
			fm["recipient"]=publickey;
			fm["sentdate"]=now.Format("%Y-%m-%d %H:%M:%S");
			rsa.Encrypt(message,fm.Body());
			data=fm.GetMessageText();

			StringFunctions::Convert(localidentityid,idstr);
			StringFunctions::Convert(data.size(),datalengthstr);
			StringFunctions::Convert(GetNextMessageIndex(localidentityid,now),indexstr);

			st=m_db->Prepare("INSERT INTO tblInsertedMessageIndex(LocalIdentityID,Date,MessageIndex) VALUES(?,?,?);");
			st.Bind(0,localidentityid);
			st.Bind(1,now.Format("%Y-%m-%d"));
			st.Bind(2,indexstr);
			st.Step();

			mess["URI"]="SSK@"+m_identitykeys[localidentityid].substr(4)+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Message-"+indexstr;
			mess["Identifier"]=m_fcpuniqueidentifier+"|"+idstr+"|"+now.Format("%Y-%m-%d")+"|"+indexstr+"|"+mess["URI"];
			mess["UploadFrom"]="direct";
			mess["DataLength"]=datalengthstr;

			m_fcp->Send(mess);
			m_fcp->Send(std::vector<char>(data.begin(),data.end()));

			m_log->Debug("FreenetMessageInserter::StartInsert started insert of private message "+mess["Identifier"]);
		}
	}

}
