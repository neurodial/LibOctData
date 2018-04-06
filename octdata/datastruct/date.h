#pragma once


#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>

#include<ostream>

#include"objectwrapper.h"

namespace OctData
{
	class Date
	{
		struct TimeCollection
		{
			time_t unixtime = 0;
			int    ms       = 0;
		};

		time_t unixtime;
		struct tm timeinfo;
		bool   decoded      = false;
		int    millisecunds = 0;


		static TimeCollection conviertWindowsTicks(long long windowsTicks);
		static TimeCollection convertWindowsTimeFormat(double wintime);
		static TimeCollection convertTime(int year, int month, int day, int hour, int min, double sec, bool withTime);
		void decodeUnixTime();

		explicit Date(time_t unixtime) : unixtime(unixtime)      { decodeUnixTime(); }
		explicit Date(const TimeCollection& time)
		: unixtime    (time.unixtime)
		, millisecunds(time.ms)
		{
			decodeUnixTime();
		}
	public:
		Date() : unixtime()                                      { }

		static Date fromWindowsTicks(uint64_t ticks)             { return Date(conviertWindowsTicks(ticks)); }
		static Date fromWindowsTimeFormat(double t)              { return Date(convertWindowsTimeFormat(t)); }
		static Date fromUnixTime(time_t unixtime)                { return Date(unixtime); }
		static Date fromDate(int year, int month, int day)       { return Date(convertTime(year, month, day, 0, 0, 0, false)); }
		static Date fromTime(int year, int month, int day, int hour, int min, double sec)
		                                                         { return Date(convertTime(year, month, day, hour, min, sec, true)); }

		int day  ()                                        const { return timeinfo.tm_mday       ; }
		int month()                                        const { return timeinfo.tm_mon  + 1   ; }
		int year ()                                        const { return timeinfo.tm_year + 1900; }
		int hour ()                                        const { return timeinfo.tm_hour       ; }
		int min  ()                                        const { return timeinfo.tm_min        ; }
		int sec  ()                                        const { return timeinfo.tm_sec        ; }
		int ms   ()                                        const { return millisecunds           ; }


		void setDay  (int value)                                 { timeinfo.tm_mday  = value       ; }
		void setMonth(int value)                                 { timeinfo.tm_mon   = value - 1   ; }
		void setYear (int value)                                 { timeinfo.tm_year  = value - 1900; }
		void setHour (int value)                                 { timeinfo.tm_hour  = value       ; }
		void setMin  (int value)                                 { timeinfo.tm_min   = value       ; }
		void setSec  (int value)                                 { timeinfo.tm_sec   = value       ; }
		void setMs   (int value)                                 { millisecunds      = value       ; }
		void setDateAsValid()                                    { decoded = true; }
		
		bool isEmpty()                                     const { return !decoded;         }

		std::string str(char trenner = '.') const;
		std::string strTime(char timeTrenner = ':') const;
		std::string strUsDate(char trenner = ':') const;
		std::string timeDateStr(char datetrenner = '.', char timeTrenner = ':', bool showMs = false) const;

		void decodeString(const std::string& str);

		void print(std::ostream& stream)                         const { stream << timeDateStr(); }
	};


	inline std::string Date::str(char trenner) const
	{
		if(!decoded)
			return "-";

		std::ostringstream datesstring;
		datesstring << year() << trenner << std::setw(2) << std::setfill('0') << month() << trenner << std::setw(2) << day();
		return datesstring.str();
	}

	inline std::string Date::strUsDate(char trenner) const
	{
		if(!decoded)
			return "-";

		std::ostringstream datesstring;
		datesstring << std::setw(2) << std::setfill('0') << month() << trenner << std::setw(2) << day() << trenner << year();
		return datesstring.str();
	}

	inline std::string Date::strTime(char timeTrenner) const
	{
		if(!decoded)
			return "-";

		std::ostringstream datesstring;
		datesstring << hour() << timeTrenner << std::setw(2) << min() << timeTrenner << std::setw(2) << sec();
		return datesstring.str();
	}

	inline std::string Date::timeDateStr(char datetrenner, char timeTrenner, bool showMs) const
	{
		if(!decoded)
			return "-";

		std::ostringstream datesstring;
		datesstring << year() << datetrenner << std::setw(2) << std::setfill('0') << month() << datetrenner << std::setw(2) << day();
		datesstring << " ";
		datesstring << hour() << timeTrenner << std::setw(2) << min() << timeTrenner << std::setw(2) << sec();
		if(showMs)
			datesstring << '.' << std::setw(3) << ms();

		return datesstring.str();
	}

	inline std::ostream& operator<<(std::ostream& stream, const Date& obj) { obj.print(stream); return stream; }

	typedef ObjectWrapper<Date> DateWrapper;
}


