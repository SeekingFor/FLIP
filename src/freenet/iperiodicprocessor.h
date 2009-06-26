#ifndef _iperiodicprocessor_
#define _iperiodicprocessor_

#include "periodicprocessorregistrar.h"

class IPeriodicProcessor
{
public:
	IPeriodicProcessor(PeriodicProcessorRegistrar *registrar)	{ registrar->Register(this); }
	virtual ~IPeriodicProcessor()								{ }

	virtual void Process()=0;
};

#endif	// _iperiodicprocessor_
