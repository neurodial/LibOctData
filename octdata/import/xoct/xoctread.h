#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	class XOctRead : public OctFileReader
	{
	public:
		XOctRead();

		virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

