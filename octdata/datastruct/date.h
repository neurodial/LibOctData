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
			int    ms = 0;
		};

		static const long long windowsTicksToUnixFactor = 10000000;
		static const long long secToUnixEpechFromWindowsTicks = 11644473600LL;

		time_t unixtime;
		struct tm timeinfo;
		bool decoded = false;
		int    millisecunds;


		static TimeCollection conviertWindowsTicks(long long windowsTicks)
		{
			TimeCollection time;
			time.unixtime = (windowsTicks/windowsTicksToUnixFactor - secToUnixEpechFromWindowsTicks);
			time.ms       = static_cast<int>((windowsTicks/(windowsTicksToUnixFactor/1000) - secToUnixEpechFromWindowsTicks*1000) - static_cast<long long>(time.unixtime)*1000);
			return time;
		}

		static TimeCollection convertWindowsTimeFormat(double wintime)
		{
			TimeCollection time;
			time.unixtime = static_cast<time_t>((wintime - 25569)*60*60*24);
			time.ms       = static_cast<int>((wintime - 25569)*60*60*24*1000 - std::floor((wintime - 25569)*60*60*24*1000));
			return time;
		}

		void decodeUnixTime()
		{
			struct tm* ti = gmtime(&unixtime);
			if(ti == nullptr)
				return;
			timeinfo = *ti;
			decoded = true;
		}

		Date(time_t unixtime) : unixtime(unixtime)               { decodeUnixTime(); }
		Date(const TimeCollection& time)
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

		std::string str(char trenner = '.') const
		{
			if(!decoded)
				return "-";

			std::ostringstream datesstring;
			datesstring << year() << trenner << std::setw(2) << std::setfill('0') << month() << trenner << std::setw(2) << day();
			return datesstring.str();
		}

		std::string timeDateStr(char datetrenner = '.', char timeTrenner = ':', bool showMs = false) const
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
	};


}
