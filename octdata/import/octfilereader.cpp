#include "octfilereader.h"

#include "../octfileread.h"


#include "import/he_vol/volread.h"
#include "import/he_xml/he_xmlread.h"
#include "cirrus_raw/cirrus_rawread.h"
#include "dicom/dicomread.h"
#include "he_e2e/he_e2eread.h"

namespace OctData
{
	OctFileReader::OctFileReader()
	{ }

	OctFileReader::OctFileReader(const OctExtension& ext)
	{
		OctFileRead& fileRead = OctFileRead::getInstance();
		fileRead.registerFileRead(this, ext);
	}

	OctFileReader::OctFileReader(const std::vector<OctExtension>& extList)
	{
		OctFileRead& fileRead = OctFileRead::getInstance();
		for(const OctExtension& ext : extList)
			fileRead.registerFileRead(this, ext);
	}

	OctFileReader::~OctFileReader()
	{

	}

	void OctFileReader::registerReaders()
	{
		VOLRead::getInstance();
		HeXmlRead::getInstance();
		CirrusRawRead::getInstance();
		DicomRead::getInstance();
		HeE2ERead::getInstance();
	}

}

