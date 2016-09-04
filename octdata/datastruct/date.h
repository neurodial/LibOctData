#pragma once


#include <sstream>
#include <iomanip>
#include <ctime>

namespace OctData
{
	class Date
	{
		static const long long windowsTicksToUnixFactor = 10000000;
		static const long long secToUnixEpechFromWindowsTicks = 11644473600LL;

		time_t unixtime;
		struct tm timeinfo;
		bool decoded = false;


		static std::time_t windowsTickToTime_t(long long windowsTicks)
		{
			std::time_t time = (windowsTicks / windowsTicksToUnixFactor - secToUnixEpechFromWindowsTicks);
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

	public:
		Date(time_t unixtime) : unixtime(unixtime)               { decodeUnixTime(); }
		Date() : unixtime()                                      { }

		static Date fromWindowsTicks(uint64_t ticks)             { return Date(windowsTickToTime_t(ticks)); }
		static Date fromWindowsTimeFormat(double t)              { return Date((t - 25569)*60*60*24); }

		int day  ()                                        const { return timeinfo.tm_mday       ; }
		int month()                                        const { return timeinfo.tm_mon  + 1   ; }
		int year ()                                        const { return timeinfo.tm_year + 1900; }
		int hour ()                                        const { return timeinfo.tm_hour; }
		int min  ()                                        const { return timeinfo.tm_min ; }
		int sec  ()                                        const { return timeinfo.tm_sec ; }

		std::string str(char trenner = '.') const
		{
			if(!decoded)
				return "-";

			std::ostringstream datesstring;
			datesstring << year() << trenner << std::setw(2) << std::setfill('0') << month() << trenner << std::setw(2) << day();
			return datesstring.str();
		}

		std::string timeDateStr(char datetrenner = '.', char timeTrenner = ':') const
		{
			if(!decoded)
				return "-";

			std::ostringstream datesstring;
			datesstring << year() << datetrenner << std::setw(2) << std::setfill('0') << month() << datetrenner << std::setw(2) << day();
			datesstring << " ";
			datesstring << hour() << timeTrenner << std::setw(2) << min() << timeTrenner << std::setw(2) << sec();
			return datesstring.str();
		}
	};


}
