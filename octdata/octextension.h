#pragma once

#include <vector>
#include <algorithm>
#include <string>

#include <initializer_list>

#include<iostream> // TODO

namespace OctData
{
	class OctExtension
	{
	public:
		typedef std::vector<std::string> ExtList;
		
		OctExtension() = default;

		OctExtension(const std::string& ext, const std::string& name)
		: extensions(1)
		, name(name)
		{
			extensions.at(0) = ext;
		}

		explicit OctExtension(const std::initializer_list<std::string>& seq)
		: extensions(seq.size()-1)
		{
			std::initializer_list<std::string>::const_iterator srcIt = seq.begin();
			ExtList::iterator destIt = extensions.begin();

			for(std::size_t i=0; i<seq.size()-1; ++i, ++srcIt, ++destIt)
				*destIt = *srcIt;

			name = *srcIt;
		}


		bool matchWithFile(const std::string& filename) const
		{
			const std::size_t fileLength = filename.length();
			std::string lowerCaseName = filename;
			std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);

			for(const std::string& str : extensions)
			{
				const std::size_t extLength = str.length();
				if(fileLength > extLength)
				{
					if(str == lowerCaseName.substr(fileLength-extLength, extLength))
						return true;
				}
			}
			return false;
		}


		ExtList extensions;
		std::string name;
	};


	class OctExtensionsList : public std::vector<OctExtension>
	{
	public:
		OctExtensionsList() = default;
		OctExtensionsList(const std::initializer_list<OctExtension>& list) : std::vector<OctExtension>(list) {};
	};
}
