#include "freenetconnection.h"
#include "../option.h"
#include "../stringfunctions.h"

#include "freenetunkeyedidentitycreator.h"
#include "freenetidentityinserter.h"
#include "freenetmessageinserter.h"
#include "freenetidentityannouncer.h"
#include "freenetnewidentityfinder.h"
#include "freenetidentityrequester.h"
#include "freenetmessagefinder.h"
#include "freenetmessagedownloader.h"
#include "freenetmessageeditionpoller.h"
#include "dbmaintenance.h"

#include <cstdlib>

FreenetConnection::FreenetConnection():m_receivednodehello(false),m_lasttriedconnecting(0),m_wasconnected(false)
{
	srand(time(0));

	m_objects.push_back(new FreenetUnkeyedIdentityCreator(this,&m_fcp));
	m_objects.push_back(new FreenetIdentityInserter(this,&m_fcp));
	m_objects.push_back(new FreenetMessageInserter(this,&m_fcp));
	m_objects.push_back(new FreenetIdentityAnnouncer(this,&m_fcp));
	m_objects.push_back(new FreenetNewIdentityFinder(this,&m_fcp));
	m_objects.push_back(new FreenetIdentityRequester(this,&m_fcp));
	m_objects.push_back(new FreenetMessageFinder(this,&m_fcp));
	m_objects.push_back(new FreenetMessageDownloader(this,&m_fcp));
	m_objects.push_back(new FreenetMessageEditionPoller(this,&m_fcp));
	m_objects.push_back(new DBMaintenance(this));

}

FreenetConnection::~FreenetConnection()
{
	for(std::vector<void *>::iterator i=m_objects.begin(); i!=m_objects.end(); i++)
	{
		delete (*i);
	}
}

const bool FreenetConnection::Connect()
{
	if(m_fcp.IsConnected()==false)
	{
		DateTime testdate;
		testdate.Add(0,-1);
		// only try connecting once every minute
		if(m_lasttriedconnecting<testdate)
		{
			Option option;
			std::string fcphost("");
			std::string fcpportstr("");
			int fcpport=-1;

			m_lasttriedconnecting.SetNowUTC();

			option.Get("FCPHost",fcphost);
			option.Get("FCPPort",fcpportstr);
			option.GetInt("FCPPort",fcpport);

			m_log->Info("FreenetConnection::Connect trying to connect to "+fcphost+":"+fcpportstr);
			if(m_fcp.Connect(fcphost,fcpport))
			{
				m_log->Info("FreenetConnection::Connect connected to node");

				m_wasconnected=true;
				m_receivednodehello=false;

				std::string randnum("");
				StringFunctions::Convert(rand(),randnum);
				std::string name("flip-"+randnum);
				m_fcp.Send(FCPv2::Message("ClientHello",2,"Name",name.c_str(),"ExpectedVersion","2.0"));

				return true;
			}
			else
			{
				m_log->Error("FreenetConnection::Connect couldn't connect to node");
				return false;
			}
		}
		else
		{
			m_receivednodehello=false;
			return false;
		}
	}
	else
	{
		m_receivednodehello=false;
		return true;
	}
}

void FreenetConnection::Update(const unsigned long ms)
{
	if(m_fcp.IsConnected())
	{
		m_fcp.Update(ms);

		if(m_fcp.MessageReady())
		{
			FCPv2::Message message;
			if(m_fcp.Receive(message))
			{
				if(m_receivednodehello==false)
				{
					if(message.GetName()=="NodeHello")
					{
						m_receivednodehello=true;
						for(std::vector<IFCPConnected *>::iterator i=m_fcpconnected.begin(); i!=m_fcpconnected.end(); i++)
						{
							(*i)->FCPConnected();
						}
						DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_FREENET_CONNECTED));
					}
					else
					{
						m_log->Error("FreenetConnection::Update expected NodeHello but received "+message.GetName());
					}
				}
				else
				{
					bool handled=false;
					for(std::vector<IFCPMessageHandler *>::iterator i=m_messagehandlers.begin(); handled==false && i!=m_messagehandlers.end(); i++)
					{
						if((*i)->HandleFCPMessage(message))
						{
							handled=true;
						}
					}
					if(handled==false)
					{
						std::string messagetext="FreenetConnection::Update received unhandled message "+message.GetName();
						for(std::map<std::string,std::string>::iterator i=message.GetFields().begin(); i!=message.GetFields().end(); i++)
						{
							messagetext+="\r\n"+(*i).first+"="+(*i).second;
						}
						m_log->Error(messagetext);
						// we must receive all data for unhandled AllData message if there is any
						if(message.GetName()=="AllData")
						{
							unsigned long datalength=0;
							StringFunctions::Convert(message["DataLength"],datalength);
							if(m_fcp.WaitForBytes(ms,datalength))
							{
								m_fcp.ReceiveIgnore(datalength);
							}
						}
					}
				}
			}
		}

		// do any periodic processing
		if(m_receivednodehello)
		{
			for(std::vector<IPeriodicProcessor *>::iterator i=m_periodicprocessors.begin(); i!=m_periodicprocessors.end(); i++)
			{
				(*i)->Process();
			}
		}

	}
	else
	{
		if(m_wasconnected==true)
		{
			for(std::vector<IFCPConnected *>::iterator i=m_fcpconnected.begin(); i!=m_fcpconnected.end(); i++)
			{
				(*i)->FCPDisconnected();
			}
			m_wasconnected=false;
			DispatchFLIPEvent(FLIPEvent(FLIPEvent::EVENT_FREENET_DISCONNECTED));
		}
		Connect();
	}
}
