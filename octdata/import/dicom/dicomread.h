#pragma once

#include <string>
#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class DicomRead : public OctFileReader
	{
	    DicomRead();

		bool readDicomDir(const boost::filesystem::path& file, OCT& oct);
	public:
		static DicomRead* getInstance();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct);
	};

}

