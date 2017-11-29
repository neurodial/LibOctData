#pragma once

#include "../octextension.h"

namespace CppFW { class Callback; }

namespace OctData
{
	class FileReadOptions;
	class OctFileRead;
	class OCT;
	class FileReader;

	class OctFileReader
	{
		OctExtensionsList extList;

	public:
		OctFileReader();
		explicit OctFileReader(const OctExtension& ext);
		explicit OctFileReader(const OctExtensionsList& ext);

		virtual ~OctFileReader();
		virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) = 0;
		const OctExtensionsList& getExtentsions() const { return extList; }

		static void registerReaders(OctFileRead& fileRead);
	};

}
