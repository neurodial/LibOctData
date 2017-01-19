#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class OctFileFormatRead : public OctFileReader
	{
		OctFileFormatRead();
	public:
		static OctFileFormatRead* getInstance();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op);
	};
}

