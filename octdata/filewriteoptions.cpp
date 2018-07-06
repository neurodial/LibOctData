
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
