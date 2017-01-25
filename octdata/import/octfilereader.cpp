#include "octfilereader.h"

#include "../octfileread.h"


#include "he_vol/volread.h"
#include "he_xml/he_xmlread.h"
#include "oct/octfileread.h"
#include "topcon/topconread.h"
#include "cirrus_raw/cirrus_rawread.h"
#include "dicom/dicomread.h"
#include "he_e2e/he_e2eread.h"
#include "tiffstack/tiffstackread.h"
#include "cvbin/cvbinread.h"

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
#ifdef HE_VOL_SUPPORT
		VOLRead::getInstance();
#endif
#ifdef HE_XML_SUPPORT
		HeXmlRead::getInstance();
#endif
#ifdef CIRRUS_RAW_SUPPORT
		CirrusRawRead::getInstance();
#endif
#ifdef DICOM_SUPPORT
		DicomRead::getInstance();
#endif
#ifdef HE_E2E_SUPPORT
		HeE2ERead::getInstance();
#endif
#ifdef TIFFSTACK_SUPPORT
		TiffStackRead::getInstance();
#endif
#ifdef CVBIN_SUPPORT
		CvBinRead::getInstance();
#endif
#ifdef OCT_FILE_SUPPORT
		OctFileFormatRead::getInstance();
#endif
#ifdef TOPCON_FILE_SUPPORT
		TopconFileFormatRead::getInstance();
#endif
	}

}

