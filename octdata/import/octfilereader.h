#pragma once

#include "../octextension.h"

namespace boost { namespace filesystem { class path; } }

namespace OctData
{
	class FileReadOptions;
	class OCT;

	class OctFileReader
	{
	public:
		OctFileReader();
		OctFileReader(const OctExtension& ext);
		OctFileReader(const std::vector<OctExtension>& ext);

		virtual ~OctFileReader();
		virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op) = 0;

		static void registerReaders();
	};

}