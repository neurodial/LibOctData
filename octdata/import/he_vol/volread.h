#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class VOLRead : public OctFileReader
	{
	public:
		VOLRead();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

