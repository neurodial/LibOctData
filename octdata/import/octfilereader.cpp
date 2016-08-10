#include "octfilereader.h"

#include "../octfileread.h"


#include "import/he_vol/volread.h"
#include "import/he_xml/he_xmlread.h"

namespace OctData
{

	OctFileReader::OctFileReader(const OctExtension& ext)
	{
		OctFileRead& fileRead = OctFileRead::getInstance();
		fileRead.registerFileRead(this, ext);
	}

	OctFileReader::~OctFileReader()
	{

	}


	OctFileReader::OctFileReader(const std::vector<OctExtension>& extList)
	{
		OctFileRead& fileRead = OctFileRead::getInstance();
		for(const OctExtension& ext : extList)
			fileRead.registerFileRead(this, ext);
	}

	void OctFileReader::registerReaders()
	{
		OctData::VOLRead::getInstance();
		OctData::HeXmlRead::getInstance();

	}

}

