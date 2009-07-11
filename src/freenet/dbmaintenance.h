#ifndef _dbmaintenance_
#define _dbmainentance_

#include "freenetconnection.h"
#include "iperiodicprocessor.h"
#include "../idatabase.h"
#include "../datetime.h"
#include "../ilogger.h"

class DBMaintenance:public IDatabase,public IPeriodicProcessor,public ILogger
{
public:
	DBMaintenance(FreenetConnection *connection);
	~DBMaintenance();

	void Process();

private:
	void Do6HoursMaintenance();
	void Do1DayMaintenance();

	DateTime m_last6hourmaintenance;
	DateTime m_last1daymaintenance;
};

#endif	// _dbmaintenance_
