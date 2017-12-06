#pragma once

#include "filereader.h"
#include <fstream>

namespace OctData
{

	class FileStreamDircet : public FileStreamInterface
	{
		std::fstream stream;
	public:
		FileStreamDircet(const boost::filesystem::path& filepath);


		virtual std::streamsize read(char* dest, std::streamsize size) override
		                                                               { stream.read(dest, size); return size; }
		virtual void seekg(std::streamoff pos) override                { stream.seekg(pos); }

		bool good()                                     const override { return stream.good(); }

	};

}
