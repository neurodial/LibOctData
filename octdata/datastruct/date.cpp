#include "date.h"

#include <cmath>

#ifdef _MSC_VER
	#define timegm _mkgmtime
#endif

namespace
{
	const long long windowsTicksToUnixFactor = 10000000;
	const long long secToUnixEpechFromWindowsTicks = 11644473600LL;
}



namespace OctData
{
	Date::TimeCollection Date::conviertWindowsTicks(long long windowsTicks)
	{
		TimeCollection time;
		time.unixtime = (windowsTicks/windowsTicksToUnixFactor - secToUnixEpechFromWindowsTicks);
		time.ms       = static_cast<int>((windowsTicks/(windowsTicksToUnixFactor/1000) - secToUnixEpechFromWindowsTicks*1000) - static_cast<long long>(time.unixtime)*1000);
		return time;
	}

	Date::TimeCollection Date::convertWindowsTimeFormat(double wintime)
	{
		TimeCollection time;
		time.unixtime = static_cast<time_t>((wintime - 25569)*60*60*24);
		time.ms       = static_cast<int>((wintime - 25569)*60*60*24*1000 - std::floor((wintime - 25569)*60*60*24*1000));
		return time;
	}

	void Date::decodeUnixTime()
	{
		struct tm* ti = gmtime(&unixtime);
		if(ti == nullptr)
			return;
		timeinfo = *ti;
		decoded = true;
	}

	Date::TimeCollection Date::convertTime(int year, int month, int day, int hour, int min, double sec, bool withTime)
	{
		struct tm timeinfo{};

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon  = month - 1;
		timeinfo.tm_mday = day;
		if(withTime)
		{
			timeinfo.tm_hour = hour;
			timeinfo.tm_min  = min;
			timeinfo.tm_sec  = static_cast<int>(sec);
		}

		TimeCollection time;
		time.unixtime = timegm(&timeinfo);
		time.ms       = static_cast<int>((sec - std::floor(sec))*1000);
		return time;
	}



}
