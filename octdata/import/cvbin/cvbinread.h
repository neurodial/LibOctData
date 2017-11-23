#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class CvBinRead : public OctFileReader
	{
	public:
		CvBinRead();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

