#include "filereader.h"

#include<boost/endian/conversion.hpp>

#include"filestreamdircet.h"
#include"filestreamgzip.h"
#include <import/platform_helper.h>

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

	std::size_t FileReader::file_size() const
	{
		if(filesize == 0)
		{
			switch(compressType)
			{
				case Compressed::none:
					filesize = bfs::file_size(filepath);
					break;
				case Compressed::gzip:
					std::ifstream stream(filepathConv(filepath), std::ios::binary | std::ios::in);
					stream.seekg(-4, std::ios_base::end);
					unsigned int size;
					stream.read(reinterpret_cast<char*>(&size), sizeof(size));
					boost::endian::little_to_native_inplace(size);
					filesize = size;
					break;
			}
		}

		return filesize;
	}




}
