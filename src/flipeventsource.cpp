#include "flipeventsource.h"
#include "flipeventhandler.h"

std::map<int,std::set<FLIPEventHandler *> > FLIPEventSource::m_flipeventhandlers;

const bool FLIPEventSource::DispatchFLIPEvent(const FLIPEvent &flipevent)
{
	if(m_flipeventhandlers.find(flipevent.GetType())!=m_flipeventhandlers.end())
	{
		for(std::set<FLIPEventHandler *>::iterator i=m_flipeventhandlers[flipevent.GetType()].begin(); i!=m_flipeventhandlers[flipevent.GetType()].end(); i++)
		{
			(*i)->HandleFLIPEvent(flipevent);
		}
		return true;
	}
	else
	{
		return false;
	}
}

const bool FLIPEventSource::RegisterFLIPEventHandler(const int eventtype,FLIPEventHandler *handler)
{
	m_flipeventhandlers[eventtype].insert(handler);
	return true;
}
