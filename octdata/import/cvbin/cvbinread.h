#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class CvBinRead : public OctFileReader
	{
	public:
		CvBinRead();

		virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

