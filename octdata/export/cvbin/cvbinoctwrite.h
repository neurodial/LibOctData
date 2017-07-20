#pragma once

namespace boost { namespace filesystem { class path; } }


namespace OctData
{
	class OCT;

	class CvBinOctWrite
	{
	public:

		static bool writeFile(const boost::filesystem::path& file, const OCT& oct);
	};
}

