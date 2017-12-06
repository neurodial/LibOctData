#pragma once

#ifdef WITH_ZLIB

#include "filereader.h"
#include <zlib.h>


namespace OctData
{

	class FileStreamGZip : public FileStreamInterface
	{
		gzFile file;
	public:
		FileStreamGZip(const boost::filesystem::path& filepath);
		virtual ~FileStreamGZip();


		virtual std::streamsize read(char* dest, std::streamsize size) override
		                                                               { return gzread(file, dest, static_cast<unsigned>(size)); }
		virtual void seekg(std::streamoff pos)                override { gzseek(file, pos, SEEK_SET); }
		virtual bool good()                             const override { return !gzeof(file); }
	};

}

#endif
