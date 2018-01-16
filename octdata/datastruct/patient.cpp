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
