#include "flipevent.h"

FLIPEvent::FLIPEvent(const int type):m_type(type)
{
	
}

FLIPEvent::FLIPEvent(const int type, const std::map<std::string,std::string> &parameters):m_type(type),m_parameters(parameters)
{
	
}

FLIPEvent::~FLIPEvent()
{

}
