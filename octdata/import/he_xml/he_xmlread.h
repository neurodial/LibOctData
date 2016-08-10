#pragma once
#include <string>

#include "../octfilereader.h"

class SLOImage;
class CScan;
class AlgoBatch;

namespace OctData
{

	class HeXmlRead : public OctFileReader
	{
		HeXmlRead();
	public:

		static HeXmlRead* getInstance();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct);
	};

}