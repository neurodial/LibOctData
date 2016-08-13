#pragma once
#include <string>

#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class HeE2ERead : public OctFileReader
	{
		HeE2ERead();

		boost::filesystem::path xmlFilename;
		boost::filesystem::path xmlPath;

	public:
		static HeE2ERead* getInstance();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct);
	};
}