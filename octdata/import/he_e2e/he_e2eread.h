#pragma once
#include <string>

#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class FileReadOptions;
	
	class HeE2ERead : public OctFileReader
	{

		boost::filesystem::path xmlFilename;
		boost::filesystem::path xmlPath;

	public:
		HeE2ERead();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;

	};
}
