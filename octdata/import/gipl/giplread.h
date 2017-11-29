#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class GIPLRead : public OctFileReader
	{
	public:
		GIPLRead();

	    virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

