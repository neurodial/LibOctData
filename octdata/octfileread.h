#pragma once

#include <vector>
#include <string>

#include "octextension.h"

#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif

namespace boost { namespace filesystem { class path; } }


namespace OctData
{
	class OCT;
	class OctFileReader;
	class FileReadOptions;

	class OctFileRead
	{
		friend class OctFileReader;
	public:
		typedef std::vector<OctExtension> ExtensionsList;
		Octdata_EXPORTS static OctFileRead& getInstance()                        { static OctFileRead instance; return instance; }

		Octdata_EXPORTS static const ExtensionsList& supportedExtensions();
		Octdata_EXPORTS static OCT openFile(const std::string& filename, const FileReadOptions& op);
		Octdata_EXPORTS static OCT openFile(const std::string& filename);

		Octdata_EXPORTS static bool isLoadable(const std::string& filename);

	private:
		OctFileRead();
		~OctFileRead();

		void registerFileRead(OctFileReader* reader, const OctExtension& ext);
		OCT openFilePrivat(const std::string& filename, const FileReadOptions& op);

		ExtensionsList extensions;

		std::vector<OctFileReader*> fileReaders;
	};
	
}

