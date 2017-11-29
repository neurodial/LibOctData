#pragma once

#include <string>
#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class FileReadOptions;

	class DicomRead : public OctFileReader
	{
		bool readDicomDir(const boost::filesystem::path& file, OCT& oct);
	public:
	    DicomRead();

	    virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};

}

