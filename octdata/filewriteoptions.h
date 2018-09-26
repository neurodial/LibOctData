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

#pragma once

#include<string>
#include"datastruct/objectwrapper.h"


namespace OctData
{
	class FileWriteOptions
	{
	public:
		enum class XoctImageFormat { png, tiff, bmp, zippedBMP };
		typedef ObjectWrapper<XoctImageFormat> XoctImageFormatEnumWrapper;


		bool            octBinFlat      = false;
		XoctImageFormat xoctImageFormat = XoctImageFormat::png;


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }

	private:
		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			XoctImageFormatEnumWrapper xoctImageFormat(p.xoctImageFormat);

			getSet("octBinFlat"     , p.octBinFlat                              );
			getSet("xoctImageFormat", static_cast<std::string&>(xoctImageFormat));
		}
	};
}

