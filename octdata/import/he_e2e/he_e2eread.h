#pragma once
#include <string>

#include "../octfilereader.h"

#include <boost/filesystem.hpp>

namespace OctData
{
	class FileReadOptions;
	
	class HeE2ERead : public OctFileReader
	{
	public:
		HeE2ERead();

		virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;

	};
}
