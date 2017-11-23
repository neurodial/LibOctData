#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class TiffStackRead : public OctFileReader
	{
	public:
		TiffStackRead();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

