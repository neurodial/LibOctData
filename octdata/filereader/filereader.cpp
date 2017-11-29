#include "filereader.h"


#include"filestreamdircet.h"
#include"filestreamgzip.h"

namespace bfs = boost::filesystem;

namespace OctData
{

	FileReader::FileReader(const boost::filesystem::path& filepath)
	: filepath(filepath)
	{
		const boost::filesystem::path ext = filepath.extension();
		if(filepath.extension() == ".gz")
		{
			compressType = Compressed::gzip;
			extension = filepath.stem().extension();
		}
		else
		{
			compressType = Compressed::none;
			extension = ext;
		}
	}

	FileReader::~FileReader()
	{
		delete fileStream;
	}

	bool OctData::FileReader::openFile()
	{
		delete fileStream;
		switch(compressType)
		{
			case Compressed::none:
				fileStream = new FileStreamDircet(filepath);
				break;
			case Compressed::gzip:
#ifdef WITH_ZLIB
				fileStream = new FileStreamGZip(filepath);
#else
				fileStream = new FileStreamDircet(filepath);
#endif
				break;
		}
		return true;
	}



}
