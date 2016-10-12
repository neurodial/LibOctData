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


	class Octdata_EXPORTS OctFileRead
	{
		friend class OctFileReader;
	public:
		typedef std::vector<OctExtension> ExtensionsList;
		static OctFileRead& getInstance()                        { static OctFileRead instance; return instance; }

		static const ExtensionsList& supportedExtensions();
		static OCT openFile(const std::string& filename, const FileReadOptions& op);
		static OCT openFile(const std::string& filename);

		static bool isLoadable(const std::string& filename);

	protected:
		void registerFileRead(OctFileReader* reader, const OctExtension& ext);
		OCT openFilePrivat(const std::string& filename, const FileReadOptions& op);

	private:
		OctFileRead();
		~OctFileRead();

		ExtensionsList extensions;

		std::vector<OctFileReader*> fileReaders;
	};
	
}

