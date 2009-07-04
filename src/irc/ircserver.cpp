#include "ircserver.h"
#include "ircnick.h"
#include "irccommandresponse.h"
#include "ircchannel.h"
#include "../option.h"
#include "../stringfunctions.h"

#include <cstring>	// memset

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
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_NEWMESSAGE,this);
	FLIPEventSource::RegisterFLIPEventHandler(FLIPEvent::EVENT_FREENET_IDENTITYINACTIVE,this);
}

IRCServer::~IRCServer()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

const bool IRCServer::HandleCommand(const IRCCommand &command, IRCClientConnection *client)
{
	std::cout << "received " << command.GetCommandString() << std::endl;

	if(command.GetCommand()=="NICK")
	{
		// for now, you can only set the nickname once before registering the connection
		if((client->Registered() & IRCClientConnection::REG_NICK)!=IRCClientConnection::REG_NICK)
		{
			if(command.GetParameters().size()>0)
			{
				if(IRCNick::IsValid(command.GetParameters()[0]))
				{
					SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, PublicKey FROM tblLocalIdentity WHERE Name=?;");
					st.Bind(0,command.GetParameters()[0]);
					st.Step();
					if(st.RowReturned()==false)
					{
						DateTime now;
						st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,DateAdded) VALUES(?,?);");
						st.Bind(0,command.GetParameters()[0]);
						st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
						st.Step(true);
						client->DBID()=st.GetLastInsertRowID();
					}
					else
					{
						st.ResultInt(0,client->DBID());
						st.ResultText(1,client->PublicKey());
					}

					client->Registered()=(client->Registered() | IRCClientConnection::REG_NICK);
					client->Nick()=command.GetParameters()[0];				
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
				// TODO - check that parameters are valid!
				if(command.GetParameters().size()>0)
				{
					client->User()=command.GetParameters()[0];
				}
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_WELCOME,client->Nick(),":Welcome to the Freenet IRC network"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_YOURHOST,client->Nick(),":Your host is "+m_servername+" running version "+FLIP_VERSION));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_CREATED,client->Nick(),":This server was created "+m_datestarted.Format("%Y-%m-%d")));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_MYINFO,client->Nick(),":Empty for now"));
				client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_USERHOST,client->Nick(),":"+client->Nick()+"=-n="+client->User()+"@freenet"));

				std::map<std::string,std::string> params;
				params["nick"]=client->Nick();
				StringFunctions::Convert(client->DBID(),params["localidentityid"]);

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
				if((*i).size()>0 && (*i)[0]=='#' && IRCChannel::IsValid((*i)))
				{
					std::string joinednicks(client->Nick());

					for(std::set<int>::iterator j=m_idchannels[(*i)].begin(); j!=m_idchannels[(*i)].end(); j++)
					{
						if(m_ids[(*j)].m_nick!="" && m_ids[(*j)].m_publickey!=client->PublicKey())
						{
							std::string idstr("");
							StringFunctions::Convert((*j),idstr);
							joinednicks+=" "+m_ids[(*j)].m_nick+"_"+idstr;
						}
					}

					client->SendCommand(IRCCommand(":"+client->Nick()+"!n="+client->User()+"@freenet JOIN :"+(*i)));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NOTOPIC,client->Nick(),(*i)+" :No topic set"));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_NAMREPLY,client->Nick()," = "+(*i)+" :"+joinednicks));
					client->SendCommand(IRCCommandResponse::MakeCommand(m_servername,IRCCommandResponse::RPL_ENDOFNAMES,client->Nick()," "+(*i)+" :End of names"));

					client->JoinedChannels().insert((*i));

					std::map<std::string,std::string> params;
					params["nick"]=client->Nick();
					StringFunctions::Convert(client->DBID(),params["localidentityid"]);
					params["channel"]=(*i);

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
				if((*i).size()>0 && (*i)[0]=='#' && IRCChannel::IsValid((*i)))
				{
					if(client->JoinedChannels().find((*i))!=client->JoinedChannels().end())
					{
						client->SendCommand(IRCCommand(":"+client->Nick()+"!n="+client->User()+"@freenet PART :"+(*i)));

						client->JoinedChannels().erase((*i));

						std::map<std::string,std::string> params;
						params["nick"]=client->Nick();
						StringFunctions::Convert(client->DBID(),params["localidentityid"]);
						params["channel"]=(*i);

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
			if(IRCChannel::IsValid(command.GetParameters()[0]))
			{
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
				StringFunctions::Convert(client->DBID(),params["localidentityid"]);
				params["message"]=message;
				params["channel"]=command.GetParameters()[0];

				DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_CHANNELMESSAGE,params));
			}
		}
	}
	else if(command.GetCommand()=="PING")
	{
		if(command.GetParameters().size()>=1 && command.GetParameters()[0]==client->Nick())
		{
			client->SendCommand(":"+m_servername+" PONG "+m_servername+" :"+client->Nick());
		}
		else if(command.GetParameters().size()==1 && command.GetParameters()[0].size()>0 && command.GetParameters()[0][0]==':')
		{
			client->SendCommand(":"+m_servername+" PONG "+m_servername+" "+command.GetParameters()[0]);
		}
	}

	return true;
}

const bool IRCServer::HandleFLIPEvent(const FLIPEvent &flipevent)
{
	if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_NEWMESSAGE)
	{
		std::map<std::string,std::string> params=flipevent.GetParameters();
		int identityid=0;
		DateTime sentdate;
		DateTime thirtyminutesago;
		DateTime fiveminutesfromnow;

		thirtyminutesago.Add(0,-30);
		fiveminutesfromnow.Add(0,5);

		StringFunctions::Convert(params["identityid"],identityid);

		if(DateTime::TryParse(params["sentdate"],sentdate))
		{

			if((m_idhassent.find(identityid)!=m_idhassent.end()) || (sentdate>=thirtyminutesago && sentdate<=fiveminutesfromnow))
			{
				SendChannelMessageToClients(identityid,params["channel"],params["message"]);
				m_idhassent.insert(identityid);
			}

		}
		else
		{
			m_log->Debug("IRCServer::HandleFLIPEvent error parsing date "+params["sentdate"]);
		}
		return true;
	}
	else if(flipevent.GetType()==FLIPEvent::EVENT_FREENET_IDENTITYINACTIVE)
	{
		// send part command to connected clients
		int identityid=0;
		std::map<std::string,std::string> params=flipevent.GetParameters();
		StringFunctions::Convert(params["identityid"],identityid);

		for(std::map<std::string,std::set<int> >::iterator i=m_idchannels.begin(); i!=m_idchannels.end(); i++)
		{
			if((*i).second.find(identityid)!=(*i).second.end())
			{
				SendPartMessageToClients(identityid,(*i).first);
				(*i).second.erase(identityid);
			}
		}
	}
	return false;
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
		// send join command for this identity to connected clients
		for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
		{
			if((*i)->PublicKey()=="")
			{
				SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
				st.Bind(0,(*i)->DBID());
				st.Step();
				if(st.RowReturned())
				{
					st.ResultText(0,(*i)->PublicKey());
				}
			}

			// don't send join message if the client is the one that sent the message
			if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
			{
				(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" JOIN "+channel));
			}

		}
	}

	// send message to connected clients
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		if((*i)->PublicKey()=="")
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
			st.Bind(0,(*i)->DBID());
			st.Step();
			if(st.RowReturned())
			{
				st.ResultText(0,(*i)->PublicKey());
			}
		}

		// don't resend the message to the client that sent it in the first place
		if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
		{
			(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" PRIVMSG "+channel+" :"+message));
		}
	}
}

void IRCServer::SendPartMessageToClients(const int identityid, const std::string &channel)
{
	std::string idstr("");
	StringFunctions::Convert(identityid,idstr);

	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		if((*i)->PublicKey()!=m_ids[identityid].m_publickey && (*i)->JoinedChannels().find(channel)!=(*i)->JoinedChannels().end())
		{
			(*i)->SendCommand(IRCCommand(":"+m_ids[identityid].m_nick+"_"+idstr+" PART "+channel));
		}
	}
}

void IRCServer::Start()
{
	Option option;
	std::vector<std::string> listenaddresses;
	std::string bindaddresses;
	std::string listenport;

	m_datestarted.SetNowUTC();

	option.Get("IRCListenPort",listenport);
	option.Get("IRCBindAddresses",bindaddresses);

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

		rval=getaddrinfo((*i).c_str(),listenport.c_str(),&hint,&result);
		if(rval==0)
		{
			for(current=result; current!=0; current=current->ai_next)
			{
				m_log->Debug("IRCServer::Start trying to create socket, bind and listen on "+(*i)+" port "+listenport);

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
							m_log->Info("IRCServer::Start started listening on "+(*i)+" port "+listenport);
							m_listensockets.push_back(sock);
						}
						else
						{
							m_log->Error("IRCServer::Start socket listen failed on "+(*i)+" port "+listenport);
							#ifdef _WIN32
							closesocket(sock);
							#else
							close(sock);
							#endif
						}
					}
					else
					{
						m_log->Error("IRCServer::Start socket bind failed on "+(*i)+" port "+listenport);
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
	if(m_listensockets.size()==0)
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
			i++;
		}
		else
		{
			std::map<std::string,std::string> params;
			params["nick"]=(*i)->Nick();
			StringFunctions::Convert((*i)->DBID(),params["localidentityid"]);

			DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_IRC_USERQUIT,params));

			m_log->Info("IRCServer::Update client connection deleted");
			delete (*i);
			i=m_clients.erase(i);
		}
	}

	tv.tv_sec=ms/1000;
	tv.tv_usec=(ms%1000)*1000;

	FD_ZERO(&readfs);
	FD_ZERO(&writefs);

	// put all listen sockets on the read fd set, as well as all the client sockets
	for(std::vector<int>::iterator i=m_listensockets.begin(); i!=m_listensockets.end(); i++)
	{
		FD_SET((*i),&readfs);
		highsock=(std::max)((*i),highsock);
	}
	for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
	{
		if((*i)->IsConnected())
		{
			FD_SET((*i)->Socket(),&readfs);
			highsock=(std::max)((*i)->Socket(),highsock);
			if((*i)->SendBufferSize()>0)
			{
				FD_SET((*i)->Socket(),&writefs);
				highsock=(std::max)((*i)->Socket(),highsock);
			}
		}
	}

	// see if data is waiting on any of the sockets
	rval=select(highsock+1,&readfs,&writefs,0,&tv);

	if(rval>0)
	{
		// check for new connections
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
					m_log->Info("IRCServer::Update new client connected");
					m_clients.push_back(new IRCClientConnection(newsock,this));
				}
			}
		}

		// check for data on existing connections, or send data
		for(std::vector<IRCClientConnection *>::iterator i=m_clients.begin(); i!=m_clients.end(); i++)
		{
			if(FD_ISSET((*i)->Socket(),&readfs))
			{
				(*i)->SocketReceive();
			}
			if((*i)->IsConnected()==true && FD_ISSET((*i)->Socket(),&writefs))
			{
				(*i)->SocketSend();
			}
		}
	}

}
