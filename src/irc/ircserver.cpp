#include "ircserver.h"
#include "ircnick.h"
#include "irccommandresponse.h"
#include "ircchannel.h"
#include "../option.h"
#include "../stringfunctions.h"
#include "../rsakeypair.h"

#include <cstring>	// memset
#include <algorithm>

//debug
#include <iostream>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>  // gcc - IPPROTO_ consts
	#include <netdb.h>       // gcc - addrinfo
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
#endif

#ifdef _WIN32
bool IRCServer::m_wsastartup=false;
#endif

IRCServer::IRCServer():m_servername("flip")
{
#ifdef _WIN32
	if(m_wsastartup==false)
	{
		WSAData wsadata;
		WSAStartup(MAKEWORD(2,2),&wsadata);
		m_wsastartup=true;
	}
#endif
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_NEWCHANNELMESSAGE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_NEWPRIVATEMESSAGE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_IDENTITYINACTIVE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_JOINCHANNEL,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_PARTCHANNEL,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_CONNECTED,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_DISCONNECTED,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_KEEPALIVE,this);
	m_sslsetup=false;
}

IRCServer::~IRCServer()
{
#ifdef _WIN32
	WSACleanup();
#endif
	ShutdownServerSSL();
}

const bool IRCServer::GetPeerDBID(IRCClientConnection *client)
{
	if(client && client->PublicKey()!="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		st.Bind(0,client->PublicKey());
		st.Step();
		if(st.RowReturned())
		{
			if(st.ResultNull(0)==false)
			{
				st.ResultInt(0,client->PeerDBID());
				return true;
			}
		}
	}
	return false;
}

void IRCServer::GetPublicKey(IRCClientConnection *client)
{
	if(client->PublicKey()=="")
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,client->LocalDBID());
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,client->PublicKey());
		}
		if(client->PublicKey()!="" && m_ids.find(client->PeerDBID())!=m_ids.end())
		{
			m_ids[client->PeerDBID()].m_publickey=client->PublicKey();
			m_ids[client->PeerDBID()].m_nick=client->Nick();
		}
	}
}

const bool IRCServer::HandleCommand(const IRCCommand &command, IRCClientConnection *client)
{
	std::cout << "received " << command.GetCommandString() << std::endl;

	GetPublicKey(client);

	if(command.GetCommand()=="NICK")
	{
		// for now, you can only set the nickname once before registering the connection
		if((client->Registered() & IRCClientConnection::REG_NICK)!=IRCClientConnection::REG_NICK)
		{
			if(command.GetParameters().size()>0)
			{
				std::string nick(command.GetParameters()[0]);
				if(nick.size()>0 && nick[0]==':')
				{
					nick.erase(0,1);
				}

				if(IRCNick::IsValid(nick))
				{
					if(NickInUse(client,nick)==false)
					{
						SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, tblLocalIdentity.PublicKey, RSAPrivateKey, tblIdentity.IdentityID FROM tblLocalIdentity LEFT JOIN tblIdentity ON tblLocalIdentity.PublicKey=tblIdentity.PublicKey WHERE tblLocalIdentity.Name=?;");
						st.Bind(0,nick);
						st.Step();
						if(st.RowReturned()==false)
						{
							RSAKeyPair rsa;
							rsa.Generate();
							DateTime now;
							st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,DateAdded,RSAPublicKey,RSAPrivateKey) VALUES(?,?,?,?);");
							st.Bind(0,nick);
							st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
							st.Bind(2,rsa.GetEncodedPublicKey());
							st.Bind(3,rsa.GetEncodedPrivateKey());
							st.Step(true);
							client->LocalDBID()=st.GetLastInsertRowID();
							client->RSAPrivateKey()=rsa.GetEncodedPrivateKey();

							st=m_db->Prepare("INSERT INTO tblIdentity(Name,RSAPublicKey,DateAdded,AddedMethod) VALUES(?,?,?,?);");
							st.Bind(0,nick);
							st.Bind(1,rsa.GetEncodedPublicKey());
							st.Bind(2,now.Format("%Y-%m-%d %H:%M:%S"));
							st.Bind(3,"Local identity");
							st.Step(true);
							client->PeerDBID()=st.GetLastInsertRowID();
						}
						else
						{
							st.ResultInt(0,client->LocalDBID());
							st.ResultText(1,client->PublicKey());
							st.ResultText(2,client->RSAPrivateKey());
							if(st.ResultNull(3)==false)
							{
								st.ResultInt(3,client->PeerDBID());
							}
						}

						if(client->PeerDBID()>0)
						{
							m_ids[client->PeerDBID()].m_lastircactivity.SetNowUTC();
							m_ids[client->PeerDBID()].m_nick=nick;
							m_ids[client->PeerDBID()].m_publickey=client->PublicKey();
						}

						client->Registered()=(client->Registered() | IRCClientConnection::REG_NICK);
						client->Nick()=nick;
					}
					else
					{
						client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_NICKNAMEINUSE,"*",":Nickname is already in use."));
					}
				}
				else
				{
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_ERRONEUSNICKNAME,"*",":Erroneous nickname"));
				}
			}
			else
			{
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_NONICKNAMEGIVEN,"*",":No nickname given"));
			}
		}
	}
	else if(command.GetCommand()=="USER")
	{
		if((client->Registered() & IRCClientConnection::REG_NICK)==IRCClientConnection::REG_NICK)
		{
			if((client->Registered() & IRCClientConnection::REG_USER)!=IRCClientConnection::REG_USER)
			{
				std::string clientcountstr("0");
				std::string channelcountstr("0");

				StringFunctions::Convert(m_clients.size(),clientcountstr);
				StringFunctions::Convert(m_idchannels.size(),channelcountstr);

				// TODO - check that parameters are valid!
				if(command.GetParameters().size()>0)
				{
					client->User()=command.GetParameters()[0];
				}
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WELCOME,client->Nick(),":Welcome to the Freenet IRC network"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_YOURHOST,client->Nick(),":Your host is "+m_servername+" running version "+FLIP_VERSION));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_CREATED,client->Nick(),":This server was created "+m_datestarted.Format("%Y-%m-%d")));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_MYINFO,client->Nick(),m_servername+" "+FLIP_VERSION+" s v"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_LUSERCHANNELS,client->Nick(),channelcountstr+" :channels formed"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_LUSERME,client->Nick(),":I have "+clientcountstr+" clients and 1 server"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_USERHOST,client->Nick(),":"+client->Nick()+"=-n="+client->User()+"@freenet"));
				// reload MOTD and send to client
				ReloadMOTD();
				if(m_motdlines.size()>0)
				{
					SendMOTDLines(client);
				}

				std::map<std::string,std::string> params;
				params["nick"]=client->Nick();
				StringFunctions::Convert(client->LocalDBID(),params["localidentityid"]);

				client->LastActivity().SetNowUTC();
				DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_USERREGISTER,params));
			}
			else
			{
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_ALREADYREGISTRED,client->Nick(),":You may not reregister"));
			}
		}
	}
	else if(command.GetCommand()=="JOIN")
	{
		if(command.GetParameters().size()>0)
		{
			std::vector<std::string> channels;
			StringFunctions::Split(command.GetParameters()[0],",",channels);

			for(std::vector<std::string>::iterator i=channels.begin(); i!=channels.end(); i++)
			{
				if((*i).size()>0 && (*i)[0]=='#' && IRCChannel::ValidName((*i)))
				{
					std::string joinednicks(client->Nick());
					IRCChannel chan;
					chan.SetName((*i));

					for(std::set<int>::iterator j=m_idchannels[chan.GetName()].begin(); j!=m_idchannels[chan.GetName()].end(); j++)
					{
						if(m_ids[(*j)].m_nick!="" && m_ids[(*j)].m_publickey!=client->PublicKey())
						{
							std::string idstr("");
							StringFunctions::Convert((*j),idstr);
							joinednicks+=" "+m_ids[(*j)].m_nick+"_"+idstr;
						}
					}

					client->SendCommand(IRCCommand(":"+client->Nick()+"!n="+client->User()+"@freenet JOIN :"+chan.GetName()));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NOTOPIC,client->Nick(),chan.GetName()+" :No topic set"));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NAMREPLY,client->Nick()," = "+chan.GetName()+" :"+joinednicks));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFNAMES,client->Nick()," "+chan.GetName()+" :End of names"));

					client->JoinedChannels().insert(chan.GetName());
					if(client->PeerDBID()>0)
					{
						SendJoinMessageToClients(client->PeerDBID(),chan.GetName());
						m_idchannels[chan.GetName()].insert(client->PeerDBID());
					}

					std::map<std::string,std::string> params;
					params["nick"]=client->Nick();
					StringFunctions::Convert(client->LocalDBID(),params["localidentityid"]);
					params["channel"]=chan.GetName();

					client->LastActivity().SetNowUTC();
					DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_JOINCHANNEL,params));
				}
			}

		}
	}
	else if(command.GetCommand()=="PART")
	{
		if(command.GetParameters().size()>0)
		{
			std::vector<std::string> channels;
			StringFunctions::Split(command.GetParameters()[0],",",channels);

			for(std::vector<std::string>::iterator i=channels.begin(); i!=channels.end(); i++)
			{
				if((*i).size()>0 && (*i)[0]=='#' && IRCChannel::ValidName((*i)))
				{
					if(client->JoinedChannels().find((*i))!=client->JoinedChannels().end())
					{
						client->SendCommand(IRCCommand(":"+client->Nick()+"!n="+client->User()+"@freenet PART :"+(*i)));

						client->JoinedChannels().erase((*i));
						if(client->PeerDBID()>0)
						{
							SendPartMessageToClients(client->PeerDBID(),(*i));
							m_idchannels[(*i)].erase(client->PeerDBID());
						}

						std::map<std::string,std::string> params;
						params["nick"]=client->Nick();
						StringFunctions::Convert(client->LocalDBID(),params["localidentityid"]);
						params["channel"]=(*i);

						client->LastActivity().SetNowUTC();
						DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_PARTCHANNEL,params));
					}
				}
			}
		}
	}
	else if(command.GetCommand()=="PRIVMSG")
	{
		if(command.GetParameters().size()>1)
		{
			// message to channel
			if(command.GetParameters()[0].size()>0 && command.GetParameters()[0][0]=='#' && IRCChannel::ValidName(command.GetParameters()[0]))
			{
				IRCChannel chan;
				std::string message("");

				chan.SetName(command.GetParameters()[0]);

				std::vector<std::string>::const_iterator i=(command.GetParameters().begin()+1);
				if((*i).size()>0 && (*i)[0]==':')
				{
					message+=(*i).substr(1);
				}
				i++;
				for(;i!=command.GetParameters().end(); i++)
				{
					message+=" "+(*i);
				}

				std::map<std::string,std::string> params;
				params["nick"]=client->Nick();
				StringFunctions::Convert(client->LocalDBID(),params["localidentityid"]);
				params["message"]=message;
				params["channel"]=chan.GetName();

				client->LastActivity().SetNowUTC();
				DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_CHANNELMESSAGE,params));
			}
			// message to another user
			else
			{
				bool sentlocal=false;
				int recipientid=0;
				std::string message("");
				std::vector<std::string>::const_iterator i=(command.GetParameters().begin()+1);
				if((*i).size()>0 && (*i)[0]==':')
				{
					message+=(*i).substr(1);
				}
				i++;
				for(;i!=command.GetParameters().end(); i++)
				{
					message+=" "+(*i);
				}

				std::map<std::string,std::string> params;
				params["nick"]=client->Nick();
				StringFunctions::Convert(client->LocalDBID(),params["localidentityid"]);
				params["message"]=message;

				std::vector<std::string> nickparts;
				StringFunctions::Split(command.GetParameters()[0],"_",nickparts);
				if(nickparts.size()>1)
				{
					params["recipientidentityid"]=nickparts[nickparts.size()-1];
					StringFunctions::Convert(params["recipientidentityid"],recipientid);
				}

				client->LastActivity().SetNowUTC();

				// see if client is connected locally to this server and send directly, otherwise insert message in Freenet
				for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
				{
					if((*i)->PeerDBID()==recipientid)
					{
						if(client->PeerDBID()>0 || GetPeerDBID(client)==true)
						{
							std::string peerdbid("");
							StringFunctions::Convert(client->PeerDBID(),peerdbid);
							(*i)->SendCommand(IRCCommand(":"+client->Nick()+"_"+peerdbid+" PRIVMSG "+(*i)->Nick()+" :"+message));
							sentlocal=true;
						}
					}
				}
				if(sentlocal==false)
				{
					if(m_ids.find(recipientid)!=m_ids.end())
					{
						DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_PRIVATEMESSAGE,params));
					}
					else
					{
						client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_NOSUCHNICK,client->Nick(),command.GetParameters()[0]+" :No such nick"));
					}
				}

			}
		}
	}
	else if(command.GetCommand()=="LIST")
	{
		// list all channels
		if(command.GetParameters().size()==0 || (command.GetParameters().size()>0 && command.GetParameters()[0].size()==0) )
		{
			for(std::map<std::string,std::set<int> >::iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
			{
				std::string countstr("0");
				StringFunctions::Convert((*i).second.size(),countstr);
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_LIST,client->Nick(),(*i).first+" "+countstr+" :"));
			}
		}
		// list only specific channels
		else
		{
			std::vector<std::string> channels;
			StringFunctions::Split(command.GetParameters()[0],",",channels);
			for(std::vector<std::string>::iterator i=channels.begin(); i!=channels.end(); i++)
			{
				if(IRCChannel::ValidName((*i))==true)
				{
					IRCChannel chan;
					chan.SetName((*i));
					std::string countstr("0");
					if(m_idchannels.find(chan.GetName())!=m_idchannels.end())
					{
						StringFunctions::Convert((*m_idchannels.find(chan.GetName())).second.size(),countstr);
					}
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_LIST,client->Nick(),chan.GetName()+" "+countstr+" :"));
				}
			}
		}
		client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_LISTEND,client->Nick(),":End of list"));
	}
	else if(command.GetCommand()=="NAMES")
	{
		std::vector<std::string> channels;
		int channelcount=0;
		IRCChannel chan;

		if(command.GetParameters().size()==0)
		{
			for(std::map<std::string,std::set<int> >::const_iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
			{
				channels.push_back((*i).first);
			}
		}
		else if(command.GetParameters().size()==1)
		{
			StringFunctions::Split(command.GetParameters()[0],",",channels);
		}

		for(std::vector<std::string>::const_iterator i=channels.begin(); i!=channels.end(); i++)
		{
			if(m_idchannels.find((*i))!=m_idchannels.end())
			{
				channelcount++;
				chan.SetName((*i));
				std::string joinednicks("");
				int nickcount=0;
				for(std::set<int>::const_iterator j=m_idchannels[(*i)].begin(); j!=m_idchannels[(*i)].end(); j++)
				{
					if(m_ids[(*j)].m_nick!="" && m_ids[(*j)].m_publickey!=client->PublicKey())
					{
						std::string idstr("");
						StringFunctions::Convert((*j),idstr);
						if(joinednicks!="")
						{
							joinednicks+=" ";
						}
						joinednicks+=m_ids[(*j)].m_nick+"_"+idstr;
						nickcount++;
					}
					else if(m_ids[(*j)].m_publickey==client->PublicKey())
					{
						if(joinednicks!="")
						{
							joinednicks+=" ";
						}
						joinednicks+=client->Nick();
						nickcount++;
					}
					if(nickcount==10)
					{
						nickcount=0;
						joinednicks="";
						client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NAMREPLY,client->Nick()," = "+chan.GetName()+" :"+joinednicks));
					}
				}
				if(nickcount>0)
				{
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NAMREPLY,client->Nick()," = "+chan.GetName()+" :"+joinednicks));
				}
			}
		}

		if(channelcount==0 || channelcount>1)
		{
			client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFNAMES,client->Nick(),":End of names"));
		}
		else
		{
			client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFNAMES,client->Nick()," "+chan.GetName()+" :End of names"));
		}

	}
	else if(command.GetCommand()=="WHOIS")
	{
		if(command.GetParameters().size()==1)
		{
			int identityid=0;
			std::string idlestr("");
			std::string nicknoid(command.GetParameters()[0]);
			std::string::size_type pos=nicknoid.find_last_of("_");
			if(pos!=std::string::npos)
			{
				nicknoid.erase(pos);
			}
			std::vector<std::string> nickparts;
			StringFunctions::Split(command.GetParameters()[0],"_",nickparts);

			if(nickparts.size()>1)
			{
				StringFunctions::Convert(nickparts[nickparts.size()-1],identityid);
			}

			if(m_ids.find(identityid)!=m_ids.end() && m_ids[identityid].m_nick==nicknoid)
			{
				int chancount=0;
				std::string chanstring("");

				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISUSER,client->Nick(),command.GetParameters()[0]+" ~"+command.GetParameters()[0]+" "+m_ids[identityid].m_publickey+" * :"+command.GetParameters()[0]));

				// send joined channels
				for(std::map<std::string,std::set<int> >::const_iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
				{
					if((*i).second.find(identityid)!=(*i).second.end())
					{
						chancount+=1;
						if(chanstring!="")
						{
							chanstring+=" ";
						}
						chanstring+=(*i).first;
					}
					if(chancount==10)
					{
						client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISCHANNELS,client->Nick(),command.GetParameters()[0]+" :"+chanstring));
						chancount=0;
						chanstring="";
					}
				}
				if(chancount>0)
				{
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISCHANNELS,client->Nick(),command.GetParameters()[0]+" :"+chanstring));
				}

				// send idle time
				DateTime now;
				now.SetNowUTC();
				StringFunctions::Convert(DateTime::DifferenceS(now,m_ids[identityid].m_lastircactivity),idlestr);
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISIDLE,client->Nick(),command.GetParameters()[0]+" "+idlestr+" :seconds idle"));

				// end whois
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFWHOIS,client->Nick(),":End of /WHOIS list."));
			}
			// client's own identity
			else if(command.GetParameters()[0]==client->Nick())
			{
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISUSER,client->Nick(),client->Nick()+" ~"+client->Nick()+" "+m_ids[identityid].m_publickey+" * :"+client->Nick()));

				int chancount=0;
				std::string chanstring("");
				for(std::set<std::string>::const_iterator i=client->JoinedChannels().begin(); i!=client->JoinedChannels().end(); i++)
				{
					chancount+=1;
					if(chanstring!="")
					{
						chanstring+=" ";
					}
					chanstring+=(*i);
					if(chancount==10)
					{
						client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISCHANNELS,client->Nick(),client->Nick()+" :"+chanstring));
						chancount=0;
						chanstring="";
					}
				}
				if(chancount>0)
				{
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISCHANNELS,client->Nick(),client->Nick()+" :"+chanstring));
				}

				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WHOISIDLE,client->Nick(),client->Nick()+" 0 :seconds idle"));

				// end whois
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFWHOIS,client->Nick(),":End of /WHOIS list."));
			}
			else
			{
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_NOSUCHNICK,client->Nick(),command.GetParameters()[0]+" :No such nick"));
			}
		}
	}
	else if(command.GetCommand()=="MOTD")
	{
		if(m_motdlines.size()>0)
		{
			SendMOTDLines(client);
		}
		else
		{
			client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_NOMOTD,client->Nick(),":No Message of the Day"));
		}
	}
	else if(command.GetCommand()=="PING")
	{
		if(command.GetParameters().size()>=1 && command.GetParameters()[0]==client->Nick())
		{
			client->SendCommand(":"+m_servername+" PONG "+m_servername+" :"+client->Nick());
		}
		else if(command.GetParameters().size()==1 && command.GetParameters()[0]==m_servername)
		{
			client->SendCommand(":"+m_servername+" PONG "+m_servername);
		}
		else if(command.GetParameters().size()>=1 && command.GetParameters()[0].size()>0)
		{
			client->SendCommand(":"+m_servername+" PONG "+m_servername+" "+command.GetParameters()[0]);
		}
	}
	else if(command.GetCommand()=="QUIT")
	{
		//Just disconnect client here - Update method will take care of cleaning up the connection
		client->Disconnect();
	}
	else
	{
		client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::ERR_UNKNOWNCOMMAND,client->Nick(),":"+command.GetCommand()+" Unknown command"));
	}

	return true;
}

const bool IRCServer::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	std::map<std::string,std::string> params=flipevent.GetParameters();
	int identityid=0;
	DateTime sentdate;
	DateTime insertday;
	DateTime thirtyminutesago;
	DateTime fiveminutesfromnow;
	DateTime now;

	if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_CONNECTED)
	{
		for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
		{
			(*i)->SendCommand(IRCCommand("NOTICE * :Freenet connection established"));
		}
		return true;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_DISCONNECTED)
	{
		for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
		{
			(*i)->SendCommand(IRCCommand("NOTICE * :Freenet connection dropped"));
		}
		return true;
	}
	else
	{
		thirtyminutesago.Add(0,-30);
		fiveminutesfromnow.Add(0,5);

		StringFunctions::Convert(params["identityid"],identityid);

		// First make sure the sent date is valid and if we haven't already seen a message from this id
		// make sure that the message is from the past 30 minutes, otherwise discard the message
		if(DateTime::TryParse(params["sentdate"],sentdate))
		{
			if(m_idhassent.find(identityid)!=m_idhassent.end() || (sentdate>=thirtyminutesago && sentdate<=fiveminutesfromnow))
			{
				m_idhassent.insert(identityid);
			}
			else
			{
				// message older than 30 minutes get silently discarded
				return false;
			}
		}
		else
		{
			m_log->Debug("IRCServer::HandleFLIPEvent error parsing date "+params["sentdate"]);
			return false;
		}

		DateTime::TryParse(params["insertday"],insertday);
		insertday.Set(insertday.Year(),insertday.Month(),insertday.Day(),0,0,0);

		// Make sure we have the name and the public key of the identity
		if(m_ids.find(identityid)==m_ids.end())
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
			st.Bind(0,identityid);
			st.Step();
			if(st.RowReturned())
			{
				st.ResultText(0,m_ids[identityid].m_nick);
				st.ResultText(1,m_ids[identityid].m_publickey);
			}
		}

		// The sent date is OK, and we do want to handle this message
		// Now make sure that the edition of the message is one after the last edition
		// we already received.  Otherwise we will queue the message up to x seconds to
		// wait for the missing edition(s)

		int thisedition=0;
		int lastedition=-1;
		StringFunctions::Convert(params["edition"],thisedition);
		if(m_ids[identityid].m_lastdayedition.find(insertday)!=m_ids[identityid].m_lastdayedition.end())
		{
			lastedition=m_ids[identityid].m_lastdayedition[insertday];
		}
		else
		{
			lastedition=thisedition-1;
		}

		if(thisedition<=lastedition+1)
		{
			ProcessFLIPEvent(identityid,flipevent);
		}
		else
		{
			m_ids[identityid].m_messagequeue.insert(idinfo::messagequeueitem(flipevent,thisedition,insertday,now));
		}

		// the identity is active, so send the active message
		std::map<std::string,std::string> activeparams;
		activeparams["identityid"]=params["identityid"];
		DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IDENTITYACTIVE,activeparams));

		return true;
	}

}

const bool IRCServer::NickInUse(IRCClientConnection *client, const std::string &nick) const
{
	for(std::vector<IRCClientConnection *>::const_iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		if((*i)!=client && (*i)->Nick()==nick)
		{
			return true;
		}
	}
	return false;
}

void IRCServer::ReloadMOTD()
{
	std::string temp("");
	Option option;
	option.Get("IRCMOTD",temp);

	m_motdlines.clear();
	if(temp.size()>0)
	{
		// convert \r\n and \r to \n before splitting lines
		temp=StringFunctions::Replace(temp,"\r\n","\n");
		temp=StringFunctions::Replace(temp,"\r","\n");
		StringFunctions::Split(temp,"\n",m_motdlines);
	}
	else
	{
		m_motdlines.clear();
	}
}

void IRCServer::ProcessFLIPEvent(const int identityid, const FLIPEvent &flipevent)
{
	DateTime sentdate;
	DateTime insertday;
	std::map<std::string,std::string> params=flipevent.GetParameters();
	int edition=0;

	DateTime::TryParse(params["insertday"],insertday);
	insertday.Set(insertday.Year(),insertday.Month(),insertday.Day(),0,0,0);
	DateTime::TryParse(params["sentdate"],sentdate);
	StringFunctions::Convert(params["edition"],edition);
	if(m_ids[identityid].m_lastdayedition[insertday]<edition)
	{
		m_ids[identityid].m_lastdayedition[insertday]=edition;
	}

	if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_KEEPALIVE)
	{
		// join and part channels as required
		if(params.find("channels")!=params.end())
		{
			std::vector<std::string> channels;
			StringFunctions::Split(params["channels"]," ",channels);

			// part channels not in list
			for(std::map<std::string,std::set<int> >::iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
			{
				if((*i).second.find(identityid)!=(*i).second.end() && std::find(channels.begin(),channels.end(),(*i).first)==channels.end())
				{
					SendPartMessageToClients(identityid,(*i).first);

					(*i).second.erase(identityid);

					m_ids[identityid].m_lastircactivity.SetNowUTC();
				}
			}

			// join channels not already joined
			for(std::vector<std::string>::const_iterator ci=channels.begin(); ci!=channels.end(); ci++)
			{
				if((*ci).size()>1 && (*ci)[0]=='#' && IRCChannel::ValidName((*ci))==true)
				{
					if(m_idchannels[(*ci)].find(identityid)==m_idchannels[(*ci)].end())
					{
						m_idchannels[(*ci)].insert(identityid);
						
						SendJoinMessageToClients(identityid,(*ci));

						m_ids[identityid].m_lastircactivity.SetNowUTC();
					}
				}
			}
		}
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_NEWCHANNELMESSAGE)
	{
		SendChannelMessageToClients(identityid,params["channel"],params["message"]);
		m_ids[identityid].m_lastircactivity.SetNowUTC();
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_NEWPRIVATEMESSAGE)
	{
		SendPrivateMessageToClients(identityid,params["recipient"],params["encryptedmessage"]);
		m_ids[identityid].m_lastircactivity.SetNowUTC();
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_IDENTITYINACTIVE)
	{
		// send part command to connected clients
		for(std::map<std::string,std::set<int> >::iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
		{
			if((*i).second.find(identityid)!=(*i).second.end())
			{
				SendPartMessageToClients(identityid,(*i).first);

				(*i).second.erase(identityid);
			}
		}
		// remove the id from our cache, it is no longer active
		m_ids.erase(identityid);
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_PARTCHANNEL)
	{
		// send part command to all connected clients in channel
		for(std::map<std::string,std::set<int> >::iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
		{
			if((*i).second.find(identityid)!=(*i).second.end())
			{
				if((*i).first==params["channel"])
				{
					SendPartMessageToClients(identityid,(*i).first);

					(*i).second.erase(identityid);
				}
			}
		}
		m_ids[identityid].m_lastircactivity.SetNowUTC();
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_JOINCHANNEL)
	{
		std::string idstr(params["identityid"]);

		// send join command to connected clients
		if(m_idchannels[params["channel"]].find(identityid)==m_idchannels[params["channel"]].end())
		{
			m_idchannels[params["channel"]].insert(identityid);

			SendJoinMessageToClients(identityid,params["channel"]);
		}
		m_ids[identityid].m_lastircactivity.SetNowUTC();
	}

}

void IRCServer::SendChannelMessageToClients(const int identityid, const std::string &channel, const std::string &message)
{
	std::string idstr("");
	StringFunctions::Convert(identityid,idstr);

	// insert sender into ids list if they aren't already there
	if(m_ids.find(identityid)==m_ids.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,m_ids[identityid].m_nick);
			st.ResultText(1,m_ids[identityid].m_publickey);
		}
	}

	// make sure identity is joined to channels
	if(m_idchannels[channel].find(identityid)==m_idchannels[channel].end())
	{
		m_idchannels[channel].insert(identityid);

		SendJoinMessageToClients(identityid,channel);
	}

	// send message to connected clients
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		GetPublicKey((*i));

		// don't resend the message to the client that sent it in the first place
		if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
		{
			(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" PRIVMSG "+channel+" :"+message));
		}
	}
}

void IRCServer::SendMOTDLines(IRCClientConnection *client)
{
	client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_MOTDSTART,client->Nick(),":- "+m_servername+" Message of the Day -"));
	for(std::vector<std::string>::iterator i=m_motdlines.begin(); i!=m_motdlines.end(); i++)
	{
		client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_MOTD,client->Nick(),":- "+(*i)));
	}
	client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFMOTD,client->Nick(),":End of Message of the Day"));
}

void IRCServer::SendPrivateMessageToClients(const int identityid, const std::string &recipient, const std::string &encryptedmessage)
{
	// insert sender into ids list if they aren't already there

	if(m_ids.find(identityid)==m_ids.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,m_ids[identityid].m_nick);
			st.ResultText(1,m_ids[identityid].m_publickey);
		}
	}

	// send message to recipient of private message
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		GetPublicKey((*i));

		if((*i)->PublicKey()==recipient)
		{
			RSAKeyPair rsa;
			std::string message("");
			std::string idstr("");

			StringFunctions::Convert(identityid,idstr);
			if(rsa.SetFromEncodedPrivateKey((*i)->RSAPrivateKey()))
			{
				if(rsa.Decrypt(encryptedmessage,message))
				{
					(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" PRIVMSG "+(*i)->Nick()+" :"+message));
				}
				else
				{
					m_log->Error("IRCServer::SendPrivateMessageToClients unable to decrypt private message");
				}
			}
			else
			{
				m_log->Error("IRCServer::SendPrivateMessageToClients unable to load private key into RSAKeyPair");
			}
		}

	}

	m_log->Debug("IRCServer::SendPrivateMessageToClients handled private message");
}

void IRCServer::SendJoinMessageToClients(const int identityid, const std::string &channel)
{
	std::string idstr("");
	StringFunctions::Convert(identityid,idstr);

	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		GetPublicKey((*i));

		// don't send join message if the client is the one that sent the message
		if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
		{
			(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" JOIN "+channel));
		}
	}
}

void IRCServer::SendPartMessageToClients(const int identityid, const std::string &channel)
{
	std::string idstr("");
	StringFunctions::Convert(identityid,idstr);

	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		GetPublicKey((*i));

		if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
		{
			(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" PART "+channel));
		}
	}
}

const bool IRCServer::SetupClientSSL(IRCClientConnection::ssl_client_info &ssl, int socket)
{
	if(m_sslsetup==true)
	{
		/*
		 * Sorted by order of preference
		 */
		int ciphersuites[] =
		{
			SSL_EDH_RSA_AES_256_SHA,
			SSL_EDH_RSA_CAMELLIA_256_SHA,
			SSL_EDH_RSA_AES_128_SHA,
			SSL_EDH_RSA_CAMELLIA_128_SHA,
			SSL_EDH_RSA_DES_168_SHA,
			SSL_RSA_AES_256_SHA,
			SSL_RSA_CAMELLIA_256_SHA,
			SSL_RSA_AES_128_SHA,
			SSL_RSA_CAMELLIA_128_SHA,
			SSL_RSA_DES_168_SHA,
			SSL_RSA_RC4_128_SHA,
			SSL_RSA_RC4_128_MD5,
			0
		};
		int ret=0;
		std::string temp("");
		std::string dhprime("");
		Option option;

		option.Get("IRCSSLDHPrime",dhprime);

		entropy_init(&ssl.m_entropy);
		ret=ctr_drbg_init(&ssl.m_ctr_drbg,entropy_func,&ssl.m_entropy,0,0);
		if(ret!=0)
		{
			StringFunctions::Convert(ret,temp);
			m_log->Error("IRCServer::SetupClientSSL couldn't initialize ctr drbg - return value = "+temp);
			return false;
		}

		ret=ssl_init(&ssl.m_ssl);
		if(ret!=0)
		{
			StringFunctions::Convert(ret,temp);
			m_log->Error("IRCServer::SetupClientSSL couldn't initialize ssl - return value = "+temp);
			return false;
		}

		ssl_set_endpoint(&ssl.m_ssl,SSL_IS_SERVER);
		ssl_set_authmode(&ssl.m_ssl,SSL_VERIFY_NONE);
		ssl_set_rng(&ssl.m_ssl,ctr_drbg_random,&ssl.m_ctr_drbg);
		ssl_set_ciphersuites(&ssl.m_ssl,ciphersuites);
		ssl_set_session(&ssl.m_ssl,0,0,&ssl.m_session);
		memset(&ssl.m_session,0,sizeof(ssl.m_session));

		ssl_set_ca_chain(&ssl.m_ssl,m_ssl.m_cert.next,0,0);
		ssl_set_own_cert(&ssl.m_ssl,&m_ssl.m_cert,&m_ssl.m_rsa);
		ssl_set_dh_param(&ssl.m_ssl,dhprime.c_str(),"4");
		
		ssl_session_reset(&ssl.m_ssl);
		ssl_set_bio(&ssl.m_ssl,net_recv,&socket,net_send,&socket);

		// we do the handshake in the client thread now because it blocks
		/*
		ret=ssl_handshake(&ssl.m_ssl);
		if(ret!=0)
		{
			StringFunctions::Convert(ret,temp);
			m_log->Error("IRCServer::SetupClientSSL couldn't handshake with client - return value = "+temp);
			return false;
		}
		*/

		return true;
	}

	return false;
}

const bool IRCServer::SetupServerSSL()
{
	if(m_sslsetup==false)
	{
		int ret=0;
		Option option;
		std::string sslcertificate("");
		std::string rsakey("");
		std::string rsapassword("");
		std::string temp("");

		option.Get("IRCSSLCertificate",sslcertificate);
		option.Get("IRCSSLRSAKey",rsakey);
		option.Get("IRCSSLRSAPassword",rsapassword);

		memset(&m_ssl.m_cert,0,sizeof(x509_cert));
		ret=x509parse_crt(&m_ssl.m_cert,(const unsigned char *)sslcertificate.c_str(),sslcertificate.size());
		if(ret!=0)
		{
			StringFunctions::Convert(ret,temp);
			m_log->Error("IRCServer::SetupServerSSL couldn't read certificate - return value = "+temp);
			return false;
		}

		rsa_init(&m_ssl.m_rsa,RSA_PKCS_V15,0);
		ret=x509parse_key(&m_ssl.m_rsa,(const unsigned char *)rsakey.c_str(),rsakey.size(),(const unsigned char *)rsapassword.c_str(),rsapassword.size());
		if(ret!=0)
		{
			StringFunctions::Convert(ret,temp);
			m_log->Error("IRCServer::SetupServerSSL couldn't read RSA key - return value = "+temp);
			x509_free(&m_ssl.m_cert);
			rsa_free(&m_ssl.m_rsa);
			return false;
		}

		m_sslsetup=true;
	}

	return m_sslsetup;
}

void IRCServer::ShutdownServerSSL()
{
	if(m_sslsetup==true)
	{
		x509_free(&m_ssl.m_cert);
		rsa_free(&m_ssl.m_rsa);

		m_sslsetup=false;
	}
}

void IRCServer::Start()
{
	Option option;
	std::string temp("");
	std::vector<std::string> listenaddresses;
	std::string bindaddresses;
	std::string listenport;
	std::string listenportssl;
	bool listenunsecure=false;
	bool listenssl=false;
	m_sslsetup=false;

	m_datestarted.SetNowUTC();

	option.GetBool("IRCListenUnsecure",listenunsecure);
	option.GetBool("IRCListenSSL",listenssl);
	option.Get("IRCListenPort",listenport);
	option.Get("IRCSSLListenPort",listenportssl);
	option.Get("IRCBindAddresses",bindaddresses);

	ReloadMOTD();

	StringFunctions::Split(bindaddresses,",",listenaddresses);

	for(std::vector<std::string>::iterator i=listenaddresses.begin(); i!=listenaddresses.end(); i++)
	{
		int sock=0;
		int rval=0;
		struct addrinfo hint,*result,*current;
		result=current=0;

		memset(&hint,0,sizeof(hint));
		hint.ai_socktype=SOCK_STREAM;
		hint.ai_protocol=IPPROTO_TCP;
		hint.ai_flags=AI_PASSIVE;

		m_log->Trace("IRCServer::Start getting address info for "+(*i));

		// bind to unsecure port
		if(listenunsecure==true)
		{
			rval=getaddrinfo((*i).c_str(),listenport.c_str(),&hint,&result);
			if(rval==0)
			{
				for(current=result; current!=0; current=current->ai_next)
				{
					m_log->Debug("IRCServer::Start trying to create socket, bind and listen on "+(*i)+" unsecure port "+listenport);

					sock=socket(current->ai_family,current->ai_socktype,current->ai_protocol);
					if(sock!=-1)
					{
						#ifndef _WIN32
						const char optval='1';
						setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
						#endif
						if(bind(sock,current->ai_addr,current->ai_addrlen)==0)
						{
							if(listen(sock,10)==0)
							{
								m_log->Info("IRCServer::Start started listening on "+(*i)+" unsecure port "+listenport);
								m_listensockets.push_back(sock);
							}
							else
							{
								m_log->Error("IRCServer::Start socket listen failed on "+(*i)+" unsecure port "+listenport);
								#ifdef _WIN32
								closesocket(sock);
								#else
								close(sock);
								#endif
							}
						}
						else
						{
							m_log->Error("IRCServer::Start socket bind failed on "+(*i)+" unsecure port "+listenport);
							#ifdef _WIN32
							closesocket(sock);
							#else
							close(sock);
							#endif
						}
					}
					else
					{
						m_log->Error("IRCServer::Start couldn't create socket on "+(*i));
					}

				}
			}
			if(result)
			{
				freeaddrinfo(result);
			}
		}
		if(listenssl==true && SetupServerSSL()==true)
		{
			rval=getaddrinfo((*i).c_str(),listenportssl.c_str(),&hint,&result);
			if(rval==0)
			{
				for(current=result; current!=0; current=current->ai_next)
				{
					m_log->Debug("IRCServer::Start trying to create socket, bind and listen on "+(*i)+" SSL port "+listenportssl);

					sock=socket(current->ai_family,current->ai_socktype,current->ai_protocol);
					if(sock!=-1)
					{
						#ifndef _WIN32
						const char optval='1';
						setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
						#endif
						if(bind(sock,current->ai_addr,current->ai_addrlen)==0)
						{
							if(listen(sock,10)==0)
							{
								m_log->Info("IRCServer::Start started listening on "+(*i)+" SSL port "+listenportssl);
								m_ssllistensockets.push_back(sock);
							}
							else
							{
								m_log->Error("IRCServer::Start socket listen failed on "+(*i)+" SSL port "+listenportssl);
								#ifdef _WIN32
								closesocket(sock);
								#else
								close(sock);
								#endif
							}
						}
						else
						{
							m_log->Error("IRCServer::Start socket bind failed on "+(*i)+" SSL port "+listenportssl);
							#ifdef _WIN32
							closesocket(sock);
							#else
							close(sock);
							#endif
						}
					}
					else
					{
						m_log->Error("IRCServer::Start couldn't create socket on "+(*i));
					}

				}
			}
			if(result)
			{
				freeaddrinfo(result);
			}
		}
	}
	if(m_listensockets.size()==0 && m_ssllistensockets.size()==0)
	{
		m_log->Fatal("IRCServer::Start couldn't start listening on any interfaces");
	}
}

void IRCServer::Shutdown()
{
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		(*i)->Disconnect();
	}
	m_clients.clear();

	for(std::vector<int>::iterator i=m_listensockets.begin(); i!=m_listensockets.end(); i++)
	{
		#ifdef _WIN32
		closesocket((*i));
		#else
		close((*i));
		#endif
	}
	m_listensockets.clear();

	m_log->Debug("IRCServer::Shutdown completed");

}

void IRCServer::Update(const unsigned long ms)
{
	int rval=0;
	fd_set readfs;
	fd_set writefs;
	struct timeval tv;
	int highsock=0;

	// delete any clients that are disconnected
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); )
	{
		if((*i)->IsConnected())
		{
			// send keepalive message if this client hasn't sent anything in the last 15 minutes
			DateTime past;
			past.Add(0,-15,0,0,0,0);
			if((*i)->LastActivity()<past)
			{
				std::string chanstring("");
				std::map<std::string,std::string> params;
				StringFunctions::Convert((*i)->LocalDBID(),params["localidentityid"]);
				(*i)->LastActivity().SetNowUTC();
				
				for(std::set<std::string>::const_iterator j=(*i)->JoinedChannels().begin(); j!=(*i)->JoinedChannels().end(); j++)
				{
					if(chanstring!="")
					{
						chanstring+=" ";
					}
					chanstring+=(*j);
				}
				params["channels"]=chanstring;

				DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_KEEPALIVE,params));
			}
			i++;
		}
		else
		{
			std::map<std::string,std::string> params;
			params["nick"]=(*i)->Nick();
			StringFunctions::Convert((*i)->LocalDBID(),params["localidentityid"]);

			//send part to all joined channels and remove from channel ids
			if((*i)->PeerDBID()>0)
			{
				for(std::set<std::string>::const_iterator j=(*i)->JoinedChannels().begin(); j!=(*i)->JoinedChannels().end(); j++)
				{
					SendPartMessageToClients((*i)->PeerDBID(),(*j));
					m_idchannels[(*j)].erase((*i)->PeerDBID());
				}
			}

			DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_USERQUIT,params));

			m_log->Info("IRCServer::Update client connection deleted");
			delete (*i);
			i=m_clients.erase(i);
		}
	}

	// process any queued events
	for(std::map<int,idinfo>::iterator i=m_ids.begin(); i!=m_ids.end(); i++)
	{
		DateTime now;
		DateTime xsecondsago;
		xsecondsago.Add(-30,0,0,0,0,0);
		if((*i).second.m_messagequeue.size()>0)
		{
			std::set<idinfo::messagequeueitem,idinfo::messagequeuecompare>::iterator mi=(*i).second.m_messagequeue.begin();
			while(mi!=(*i).second.m_messagequeue.end() && ((*mi).m_edition<=(*i).second.m_lastdayedition[(*mi).m_insertday] || (*mi).m_arrivaltime<xsecondsago))
			{
				ProcessFLIPEvent((*i).first,(*mi).m_message);
				(*i).second.m_messagequeue.erase(mi);
				// gcc doesn't like assigning an iterator when erasing, so we have to do it this way
				mi=(*i).second.m_messagequeue.begin();
			}
		}
	}

	tv.tv_sec=ms/1000;
	tv.tv_usec=(ms%1000)*1000;

	FD_ZERO(&readfs);
	FD_ZERO(&writefs);

	for(std::vector<int>::iterator i=m_listensockets.begin(); i!=m_listensockets.end(); i++)
	{
		FD_SET((*i),&readfs);
		highsock=(std::max)((*i),highsock);
	}
	for(std::vector<int>::iterator i=m_ssllistensockets.begin(); i!=m_ssllistensockets.end(); i++)
	{
		FD_SET((*i),&readfs);
		highsock=(std::max)((*i),highsock);
	}

	// see if data is waiting on any of the sockets
	rval=select(highsock+1,&readfs,&writefs,0,&tv);

	if(rval>0)
	{
		// check for new connections on unsecure sockets
		for(std::vector<int>::iterator i=m_listensockets.begin(); i!=m_listensockets.end(); i++)
		{
			if(FD_ISSET((*i),&readfs))
			{
				int newsock=0;
				struct sockaddr_storage addr;
				socklen_t addrlen=sizeof(addr);
				newsock=accept((*i),(struct sockaddr *)&addr,&addrlen);
				if(newsock!=-1)
				{
					m_log->Info("IRCServer::Update new client connected on unsecure socket");
					m_clients.push_back(new IRCClientConnection(newsock,this,IRCClientConnection::CON_UNSECURE,0));
				}
			}
		}

		// check for new connectiosn on SSL sockets
		for(std::vector<int>::iterator i=m_ssllistensockets.begin(); i!=m_ssllistensockets.end(); i++)
		{
			if(FD_ISSET((*i),&readfs))
			{
				int newsock=0;
				struct sockaddr_storage addr;
				socklen_t addrlen=sizeof(addr);
				newsock=accept((*i),(struct sockaddr *)&addr,&addrlen);
				if(newsock!=-1)
				{
					IRCClientConnection::ssl_client_info *ssl=new IRCClientConnection::ssl_client_info;
					memset(ssl,0,sizeof(IRCClientConnection::ssl_client_info));
					if(ssl && SetupClientSSL(*ssl,newsock))
					{
						m_log->Info("IRCServer::Update new client connected on SSL socket");
						m_clients.push_back(new IRCClientConnection(newsock,this,IRCClientConnection::CON_SSL,ssl));
					}
					else
					{
						m_log->Error("RCServer::Update couldn't setup SSL connection");
						if(ssl)
						{
							delete ssl;
						}
					}
				}
			}
		}
	}

	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		(*i)->HandleReceivedData();
	}

}
