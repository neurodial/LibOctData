#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class OctFileFormatRead : public OctFileReader
	{
	public:
		OctFileFormatRead();

	    virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

