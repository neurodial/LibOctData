#pragma once

#include <vector>
#include <string>

namespace OctData
{
	struct OctExtension
	{
		OctExtension(const std::string& ext, const std::string& name)
		: extensions(1)
		, name(name)
		{
			extensions.at(0) = ext;
		}


		bool operator==(const std::string& ext) const
		{
			for(const std::string& str : extensions)
				if(str == ext)
					return true;
			return false;
		}

		std::vector<std::string> extensions;
		std::string name;
	};
}
