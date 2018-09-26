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


#include"filewriteoptions.h"


namespace OctData
{
	template<> void FileWriteOptions::XoctImageFormatEnumWrapper::toString()
	{
		switch(obj)
		{
			case FileWriteOptions::XoctImageFormat::png      : std::string::operator=("png"      ); break;
			case FileWriteOptions::XoctImageFormat::tiff     : std::string::operator=("tiff"     ); break;
			case FileWriteOptions::XoctImageFormat::bmp      : std::string::operator=("bmp"      ); break;
			case FileWriteOptions::XoctImageFormat::zippedBMP: std::string::operator=("zippedBMP"); break;
		}
	}

	template<> void FileWriteOptions::XoctImageFormatEnumWrapper::fromString()
	{
		     if(*this == "png"      ) obj = FileWriteOptions::XoctImageFormat::png      ;
		else if(*this == "tiff"     ) obj = FileWriteOptions::XoctImageFormat::tiff     ;
		else if(*this == "bmp"      ) obj = FileWriteOptions::XoctImageFormat::bmp      ;
		else if(*this == "zippedBMP") obj = FileWriteOptions::XoctImageFormat::zippedBMP;
	}
}
