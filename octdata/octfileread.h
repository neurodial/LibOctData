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

namespace CppFW { class Callback; }

namespace OctData
{
	class OCT;
	class OctFileReader;
	class FileReadOptions;
	class OctExtensionsList;
	class FileReader;

	class OctFileRead
	{
		friend class OctFileReader;
	public:
		Octdata_EXPORTS static OctFileRead& getInstance()                        { static OctFileRead instance; return instance; }

		Octdata_EXPORTS static const OctExtensionsList& supportedExtensions();
		Octdata_EXPORTS static OCT openFile(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback = nullptr);
		Octdata_EXPORTS static OCT openFile(const boost::filesystem::path& filename, const FileReadOptions& op, CppFW::Callback* callback = nullptr);
		Octdata_EXPORTS static OCT openFile(const std::string& filename, CppFW::Callback* callback = nullptr);

		Octdata_EXPORTS static bool isLoadable(const std::string& filename);

		Octdata_EXPORTS static void writeFile(const std::string& filename, const OCT& octdata);

	private:
		OctFileRead();
		~OctFileRead();

		void registerFileRead(OctFileReader* reader);
		OCT openFilePrivat(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback);
		OCT openFilePrivat(const boost::filesystem::path& file, const FileReadOptions& op, CppFW::Callback* callback);

		void writeFilePrivat(const std::string& filename, const OCT& octdata);

		bool openFileFromExt(OCT& oct, FileReader& filename, const FileReadOptions& op, CppFW::Callback* callback);
		bool tryOpenFile(OCT& oct, FileReader& filename, const FileReadOptions& op, CppFW::Callback* callback);

		OctExtensionsList extensions;

		std::vector<OctFileReader*> fileReaders;
	};
	
}

