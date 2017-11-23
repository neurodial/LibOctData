#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class GIPLRead : public OctFileReader
	{
	public:
		GIPLRead();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

