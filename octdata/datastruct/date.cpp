/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "date.h"

#include<cmath>
// #include<boost/spirit/include/qi.hpp>
// #include<boost/phoenix/phoenix.hpp>

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

	void Date::decodeString(const std::string& str)
	{
		unixtime = 0;
		decodeUnixTime();
		decoded = false;

		if(str.empty() || str == "-")
			return;

		std::istringstream datesstring(str);

		int year  = -1;
		int month = -1;
		int day   = -1;
		char trenner;
		datesstring >> year >> trenner >> month >> trenner >> day;
		if(day < 0)
			return;

		decoded = true;
		setYear(year);
		setMonth(month);
		setDay(day);

		int hour = -1;
		int min  = -1;
		int sec  = -1;
		datesstring >> hour >> trenner >> min >> trenner >> sec;
		if(sec < 0)
			return;

		setHour(hour);
		setMin(min);
		setSec(sec);

		int ms = -1;
		datesstring >> trenner >> ms;
		if(trenner == '.' && ms > 0)
			setMs(ms);
	}


	template<> void DateWrapper::toString()
	{
		if(!obj.isEmpty())
			std::string::operator=(obj.timeDateStr());
	}


	template<> void DateWrapper::fromString()
	{
		if(!empty())
			obj.decodeString(*this);
	}



}
