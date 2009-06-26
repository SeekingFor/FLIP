#include "datetime.h"

#include <vector>

#ifndef _WIN32
	#include <librock/datime.h>
#endif

DateTime::DateTime()
{
	SetNowUTC();
}

DateTime::DateTime(const time_t timet)
{
	Set(timet);
}

DateTime::DateTime(const int year, const int month, const int day, const int hour, const int minute, const int second)
{
	Set(year,month,day,hour,minute,second);
}

void DateTime::Add(const int seconds, const int minutes, const int hours, const int days, const int months, const int years)
{
	m_tm.tm_sec+=seconds;
	m_tm.tm_min+=minutes;
	m_tm.tm_hour+=hours;
	m_tm.tm_mday+=days;
	m_tm.tm_mon+=months;
	m_tm.tm_year+=years;
	m_tm.tm_isdst=0;
#ifdef _WIN32
	_mkgmtime(&m_tm);
#else
	time_t temp;
	librock_mkgmtime((const librock_STRUCT_TM_s *)&m_tm,(librock_TIME_T_s *)&temp);
#endif
}

const std::string DateTime::Format(const std::string &format) const
{
	std::vector<char> buffer(2048,0);
	size_t rval=strftime(&buffer[0],buffer.size()-1,format.c_str(),&m_tm);

	if(rval>0)
	{
		return std::string(buffer.begin(),buffer.begin()+rval);
	}
	else
	{
		return std::string("");
	}
}

void DateTime::Set(const time_t timet)
{
	m_tm=*gmtime(&timet);
}

void DateTime::Set(const int year, const int month, const int day, const int hour, const int minute, const int second)
{
	m_tm.tm_year=year-1900;
	m_tm.tm_mon=month-1;
	m_tm.tm_mday=day;
	m_tm.tm_hour=hour;
	m_tm.tm_min=minute;
	m_tm.tm_sec=second;
	m_tm.tm_isdst=0;
#ifdef _WIN32
	_mkgmtime(&m_tm);
#else
	time_t temp;
	librock_mkgmtime((const librock_STRUCT_TM_s *)&m_tm,(librock_TIME_T_s *)&temp);
#endif
}

void DateTime::SetNowUTC()
{
	time_t timet;
	time(&timet);
	m_tm=*gmtime(&timet);
}

const bool DateTime::operator==(const DateTime &rhs) const
{
	return (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon==rhs.m_tm.tm_mon && m_tm.tm_mday==rhs.m_tm.tm_mday && m_tm.tm_hour==rhs.m_tm.tm_hour && m_tm.tm_min==rhs.m_tm.tm_min && m_tm.tm_sec==rhs.m_tm.tm_sec);
}

const bool DateTime::operator<(const DateTime &rhs) const
{
	return ((m_tm.tm_year<rhs.m_tm.tm_year) || (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon<rhs.m_tm.tm_mon) || (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon==rhs.m_tm.tm_mon && m_tm.tm_mday<rhs.m_tm.tm_mday) || (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon==rhs.m_tm.tm_mon && m_tm.tm_mday==rhs.m_tm.tm_mday && m_tm.tm_hour<rhs.m_tm.tm_hour) || (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon==rhs.m_tm.tm_mon && m_tm.tm_mday==rhs.m_tm.tm_mday && m_tm.tm_hour==rhs.m_tm.tm_hour && m_tm.tm_min<rhs.m_tm.tm_min) || (m_tm.tm_year==rhs.m_tm.tm_year && m_tm.tm_mon==rhs.m_tm.tm_mon && m_tm.tm_mday==rhs.m_tm.tm_mday && m_tm.tm_hour==rhs.m_tm.tm_hour && m_tm.tm_min==rhs.m_tm.tm_min && m_tm.tm_sec<rhs.m_tm.tm_sec));
}

const bool DateTime::operator<=(const DateTime &rhs) const
{
	return (*this<rhs || *this==rhs);
}

const bool DateTime::operator>(const DateTime &rhs) const
{
	return !(*this<=rhs);
}

const bool DateTime::operator>=(const DateTime &rhs) const
{
	return !(*this<rhs);
}
