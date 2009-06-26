#ifndef _ilogger_
#define _ilogger_

#include "global.h"

class ILogger
{
public:
	ILogger():m_log(&global::log)	{}
protected:
	LogFile *m_log;
};

#endif	// _ilogger_
