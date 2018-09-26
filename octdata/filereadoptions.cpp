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

#include"filereadoptions.h"


namespace OctData
{
	template<> void FileReadOptions::E2eGrayTransformEnumWrapper::toString()
	{
		switch(obj)
		{
			case FileReadOptions::E2eGrayTransform::nativ: std::string::operator=("nativ"); break;
			case FileReadOptions::E2eGrayTransform::xml  : std::string::operator=("xml"  ); break;
			case FileReadOptions::E2eGrayTransform::vol  : std::string::operator=("vol"  ); break;
			case FileReadOptions::E2eGrayTransform::u16  : std::string::operator=("u16"  ); break;
		}
	}

	template<> void FileReadOptions::E2eGrayTransformEnumWrapper::fromString()
	{
		     if(*this == "nativ") obj = FileReadOptions::E2eGrayTransform::nativ;
		else if(*this == "xml"  ) obj = FileReadOptions::E2eGrayTransform::xml;
		else if(*this == "vol"  ) obj = FileReadOptions::E2eGrayTransform::vol;
		else if(*this == "u16"  ) obj = FileReadOptions::E2eGrayTransform::u16;
		else obj = FileReadOptions::E2eGrayTransform::xml;
	}
}
