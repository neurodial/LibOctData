#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class CvBinRead : public OctFileReader
	{
		CvBinRead();
	public:
		static CvBinRead* getInstance();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback);
	};
}

