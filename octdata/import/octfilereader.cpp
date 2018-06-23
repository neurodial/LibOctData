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
#include "gipl/giplread.h"
#include "xoct/xoctread.h"

namespace OctData
{
	OctFileReader::OctFileReader()
	{ }

	OctFileReader::OctFileReader(const OctExtension& ext)
	{
		extList.push_back(ext);
	}

	OctFileReader::OctFileReader(const OctExtensionsList& extList)
	: extList(extList)
	{
	}

	OctFileReader::~OctFileReader()
	{

	}

	void OctFileReader::registerReaders(OctFileRead& fileRead)
	{
#ifdef HE_VOL_SUPPORT
		fileRead.registerFileRead(new VOLRead);
#endif
#ifdef HE_XML_SUPPORT
		fileRead.registerFileRead(new HeXmlRead);
#endif
#ifdef CIRRUS_RAW_SUPPORT
		fileRead.registerFileRead(new CirrusRawRead);
#endif
#ifdef DICOM_SUPPORT
		fileRead.registerFileRead(new DicomRead);
#endif
#ifdef HE_E2E_SUPPORT
		fileRead.registerFileRead(new HeE2ERead);
#endif
#ifdef TIFFSTACK_SUPPORT
		fileRead.registerFileRead(new TiffStackRead);
#endif
#ifdef CVBIN_SUPPORT
		fileRead.registerFileRead(new CvBinRead);
#endif
#ifdef OCT_FILE_SUPPORT
		fileRead.registerFileRead(new OctFileFormatRead);
#endif
#ifdef TOPCON_FILE_SUPPORT
		fileRead.registerFileRead(new TopconFileFormatRead);
#endif
#ifdef GIPL_SUPPORT
		fileRead.registerFileRead(new GIPLRead);
#endif
#ifdef XOCT_SUPPORT
		fileRead.registerFileRead(new XOctRead);
#endif
	}

}

