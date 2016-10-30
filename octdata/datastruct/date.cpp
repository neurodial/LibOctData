#include "date.h"

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


	std::string Date::str(char trenner) const
	{
		if(!decoded)
			return "-";

		std::ostringstream datesstring;
		datesstring << year() << trenner << std::setw(2) << std::setfill('0') << month() << trenner << std::setw(2) << day();
		return datesstring.str();
	}

	std::string Date::timeDateStr(char datetrenner, char timeTrenner, bool showMs) const
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
}
