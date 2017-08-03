#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class GIPLRead : public OctFileReader
	{
		GIPLRead();
	public:
		static GIPLRead* getInstance();

	    virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback);
	};
}

