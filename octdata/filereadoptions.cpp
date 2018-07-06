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
