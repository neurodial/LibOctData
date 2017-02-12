#pragma once

#include <string>
#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class FileReadOptions;

	class DicomRead : public OctFileReader
	{
	    DicomRead();

		bool readDicomDir(const boost::filesystem::path& file, OCT& oct);
	public:
		static DicomRead* getInstance();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback);
	};

}

