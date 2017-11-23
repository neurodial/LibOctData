#pragma once

#include "../octextension.h"

namespace boost { namespace filesystem { class path; } }

namespace CppFW { class Callback; }

namespace OctData
{
	class FileReadOptions;
	class OctFileRead;
	class OCT;

	class OctFileReader
	{
		OctExtensionsList extList;

	public:
		OctFileReader();
		explicit OctFileReader(const OctExtension& ext);
		explicit OctFileReader(const OctExtensionsList& ext);

		virtual ~OctFileReader();
		virtual bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) = 0;
		const OctExtensionsList& getExtentsions() const { return extList; }

		static void registerReaders(OctFileRead& fileRead);
	};

}
