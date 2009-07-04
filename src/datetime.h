#ifndef _datetime_
#define _datetime_

#include <ctime>
#include <string>

class DateTime
{
public:
	DateTime();
	DateTime(const time_t timet);
	DateTime(const int year, const int month, const int day, const int hour, const int minute, const int second);

	void SetNowUTC();
	void Set(const time_t timet);
	void Set(const int year, const int month, const int day, const int hour, const int minute, const int second);

	void Add(const int seconds, const int minutes=0, const int hours=0, const int days=0, const int months=0, const int years=0);

	void StripTime()			{ Set(Year(),Month(),Day(),0,0,0); }

	const int Year() const		{ return m_tm.tm_year+1900; }
	const int Month() const		{ return m_tm.tm_mon+1; }
	const int Day() const		{ return m_tm.tm_mday; }
	const int Hour() const		{ return m_tm.tm_hour; }
	const int Minute() const	{ return m_tm.tm_min; }
	const int Second() const	{ return m_tm.tm_sec; }

	const std::string Format(const std::string &format) const;

	const bool operator==(const DateTime &rhs) const;
	const bool operator<(const DateTime &rhs) const;
	const bool operator<=(const DateTime &rhs) const;
	const bool operator>(const DateTime &rhs) const;
	const bool operator>=(const DateTime &rhs) const;

	static const bool TryParse(const std::string &datestring, DateTime &date);

private:
	static const bool Convert(const std::string &input, int &output);

	struct tm m_tm;
};

#endif	// _datetime_
