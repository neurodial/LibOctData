#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class TiffStackRead : public OctFileReader
	{
	public:
		TiffStackRead();

	    virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

