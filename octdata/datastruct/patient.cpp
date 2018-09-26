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

#include"patient.h"


namespace OctData
{
	const char* OctData::Patient::getSexName(Sex sex)
	{
		switch(sex)
		{
			case Sex::Female : return "Female";
			case Sex::Male   : return "Male";
			case Sex::Unknown: return "Unknown";
		}
		return "Unknown";
	}





	template<> void Patient::SexEnumWrapper::toString()
	{
		switch(obj)
		{
			case Patient::Sex::Female: std::string::operator=("female"); break;
			case Patient::Sex::Male  : std::string::operator=("male"  ); break;
			case Patient::Sex::Unknown:
				break;
		}
	}


	template<> void Patient::SexEnumWrapper::fromString()
	{
		     if(*this == "female") obj = Patient::Sex::Female;
		else if(*this == "male"  ) obj = Patient::Sex::Male  ;
		else obj = Patient::Sex::Unknown;
	}

}
