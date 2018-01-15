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

}
