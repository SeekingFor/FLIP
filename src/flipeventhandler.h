#ifndef _flipeventhandler_
#define _flipeventhandler_

#include "flipevent.h"

class FLIPEventHandler
{
public:

	virtual const bool HandleFLIPEvent(const FLIPEvent &flipevent)=0;

};

#endif	// _flipeventhandler_
