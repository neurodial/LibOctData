#include "filestreamgzip.h"


#ifdef WITH_ZLIB

namespace OctData
{

	FileStreamGZip::FileStreamGZip(const boost::filesystem::path& filepath)
	{
		file = gzopen(filepath.generic_string().c_str(), "r");
		gzbuffer(file, 128*1024);
	}

	FileStreamGZip::~FileStreamGZip()
	{
		gzclose(file);
	};

}

#endif
