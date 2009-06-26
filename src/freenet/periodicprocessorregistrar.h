#ifndef _periodicprocessorregistrar_
#define _periodicprocessorregistrar_

#include <vector>

class IPeriodicProcessor;

class PeriodicProcessorRegistrar
{
public:
	virtual ~PeriodicProcessorRegistrar()			{ }

	void Register(IPeriodicProcessor *processor)	{ m_periodicprocessors.push_back(processor); }

protected:
	std::vector<IPeriodicProcessor *> m_periodicprocessors;
};

#endif	// _periodicprocessorregistrar_
