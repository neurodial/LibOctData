#pragma once

#include <vector>
#include <string>

#include <initializer_list>

namespace OctData
{
	struct OctExtension
	{
		typedef std::vector<std::string> ExtList;

		OctExtension(const std::string& ext, const std::string& name)
		: extensions(1)
		, name(name)
		{
			extensions.at(0) = ext;
		}

		OctExtension(const std::initializer_list<std::string>& seq)
		: extensions(seq.size()-1)
		{
			std::initializer_list<std::string>::const_iterator srcIt = seq.begin();
			ExtList::iterator destIt = extensions.begin();

			for(std::size_t i=0; i<seq.size()-1; ++i, ++srcIt, ++destIt)
				*destIt = *srcIt;

			name = *srcIt;
		}


		bool operator==(const std::string& ext) const
		{
			for(const std::string& str : extensions)
				if(str == ext)
					return true;
			return false;
		}

		ExtList extensions;
		std::string name;
	};
}
