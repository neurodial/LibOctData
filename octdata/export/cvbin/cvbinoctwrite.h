#pragma once

namespace boost { namespace filesystem { class path; } }


namespace OctData
{
	class OCT;
	class Study;
	class Series;
	class Patient;

	class CvBinOctWrite
	{
	public:

		static bool writeFile(const boost::filesystem::path& file, const OCT& oct);
		static bool writeFile(const boost::filesystem::path& file, const Patient& pat, const Study& study, const Series& series);
	};
}

