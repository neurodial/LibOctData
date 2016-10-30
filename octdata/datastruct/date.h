#pragma once


#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>

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

		int day  ()                                        const { return timeinfo.tm_mday       ; }
		int month()                                        const { return timeinfo.tm_mon  + 1   ; }
		int year ()                                        const { return timeinfo.tm_year + 1900; }
		int hour ()                                        const { return timeinfo.tm_hour; }
		int min  ()                                        const { return timeinfo.tm_min ; }
		int sec  ()                                        const { return timeinfo.tm_sec ; }
		int ms   ()                                        const { return millisecunds    ; }
		
		bool isEmpty()                                     const { return !decoded;         }

		std::string str(char trenner = '.') const;
		std::string timeDateStr(char datetrenner = '.', char timeTrenner = ':', bool showMs = false) const;
	};


}
