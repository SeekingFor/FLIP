#ifndef _flipeventsource_
#define _flipeventsource_

#include <map>
#include <set>

#include "flipevent.h"
class FLIPEventHandler;

class FLIPEventSource
{
public:

	static const bool RegisterFLIPEventHandler(const int eventtype,FLIPEventHandler *handler);
	const bool DispatchFLIPEvent(const FLIPEvent &flipevent);

private:

	static std::map<int,std::set<FLIPEventHandler *> > m_flipeventhandlers;

};

#endif	// _flipeventsource_
