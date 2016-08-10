#pragma once

#include <vector>
#include <string>

namespace OctData
{
	struct OctExtension
	{
		OctExtension(const std::string& ext, const std::string& name)
		: extension(ext)
		, name(name)
		{}

		bool operator==(const std::string& ext) const { return extension == ext; }

		std::string extension;
		std::string name;
	};
}
