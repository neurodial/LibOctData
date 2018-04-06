#pragma once

namespace boost { namespace filesystem { class path; } }


namespace OctData
{
	class OCT;
	class Study;
	class Series;
	class Patient;
	class FileWriteOptions;

	class CirrusRawExport
	{
	public:
		static bool writeFile(const boost::filesystem::path& file, const OCT& oct, const FileWriteOptions& opt);
		static bool writeFile(const boost::filesystem::path& file, const OCT& oct, const Patient& pat, const Study& study, const Series& series, const FileWriteOptions& opt);
	};
}
