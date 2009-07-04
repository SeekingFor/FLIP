#include "datetime.h"

#include <vector>
#include <sstream>

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

const bool DateTime::Convert(const std::string &input, int &output)
{
	std::istringstream i(input);
	if(i>>output)
	{
		return true;
	}
	else
	{
		return false;
	}
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

const bool DateTime::TryParse(const std::string &datestring, DateTime &date)
{
	int year=0;
	int month=0;
	int day=0;
	int hour=0;
	int minute=0;
	int second=0;
	bool foundyear=false;
	bool foundmonth=false;
	bool foundday=false;
	bool foundhour=false;
	bool foundminute=false;
	bool foundsecond=false;

	std::vector<std::string> dateparts;
	std::vector<std::string>::size_type datepos=0;
	std::string::size_type offset=0;
	std::string::size_type delimIndex=0;
    
	delimIndex=datestring.find_first_of("-/ :",offset);

    while(delimIndex!=std::string::npos)
    {
        dateparts.push_back(datestring.substr(offset,delimIndex-offset));
        offset+=delimIndex-offset+1;
		delimIndex=datestring.find_first_of("-/ :", offset);
    }

    dateparts.push_back(datestring.substr(offset));

	for(datepos=0; datepos<dateparts.size(); )
	{
		if(dateparts[datepos].size()==4 && foundyear==false)
		{
			if(Convert(dateparts[datepos],year))
			{
				foundyear=true;
			}
			datepos++;
		}
		else if((dateparts[datepos].size()==1 || dateparts[datepos].size()==2))
		{
			if(foundyear==true && foundmonth==false)
			{
				if(Convert(dateparts[datepos],month))
				{
					if(month>=1 && month<=12)
					{
						foundmonth=true;
					}
					else
					{
						foundmonth=0;
					}
				}
			}
			else if(foundyear==true && foundmonth==true && foundday==false)
			{
				if(Convert(dateparts[datepos],day))
				{
					if(day>=1 && day<=31)
					{
						foundday=true;
					}
					else
					{
						foundday=0;
					}
				}
			}
			else if(foundyear==true && foundmonth==true && foundday==true && foundhour==false)
			{
				if(Convert(dateparts[datepos],hour))
				{
					if(hour>=0 && hour<=24)
					{
						foundhour=true;
					}
					else
					{
						foundhour=0;
					}
				}
			}
			else if(foundyear==true && foundmonth==true && foundday==true && foundhour==true && foundminute==false)
			{
				if(Convert(dateparts[datepos],minute))
				{
					if(minute>=0 && minute<60)
					{
						foundminute=true;
					}
					else
					{
						foundminute=0;
					}
				}
			}
			else if(foundyear==true && foundmonth==true && foundday==true && foundhour==true && foundminute==true && foundsecond==false)
			{
				if(Convert(dateparts[datepos],second))
				{
					if(second>=0 && second<60)
					{
						foundsecond=true;
					}
					else
					{
						foundsecond=0;
					}
				}
			}
			datepos++;
		}
		else
		{
			datepos++;
		}
	}

	if(foundyear==true && foundmonth==true && foundday==true)
	{
		date.Set(year,month,day,hour,minute,second);
		return true;
	}
	else
	{
		return false;
	}

}
