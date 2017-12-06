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
		static std::string toLower(const std::string& str)
		{
			std::string lowerCaseStr = str;
			std::transform(lowerCaseStr.begin(), lowerCaseStr.end(), lowerCaseStr.begin(), ::tolower);
			return lowerCaseStr;
		}
	public:
		typedef std::vector<std::string> ExtList;
		
		OctExtension() = default;

		OctExtension(const std::string& ext, const std::string& name)
		: extensions(1)
		, name(name)
		, extensionsLowerCase(1)
		{
			extensions.at(0) = ext;
			extensionsLowerCase.at(0) = toLower(ext);
		}

		explicit OctExtension(const std::initializer_list<std::string>& seq)
		: extensions(seq.size()-1)
		, extensionsLowerCase(seq.size()-1)
		{
			std::initializer_list<std::string>::const_iterator srcIt = seq.begin();
			ExtList::iterator destIt = extensions.begin();
			ExtList::iterator destLowerIt = extensionsLowerCase.begin();

			for(std::size_t i=0; i<seq.size()-1; ++i)
			{
				*destIt = *srcIt;
				*destLowerIt = toLower(*srcIt);
				++srcIt;
				++destIt;
				++destLowerIt;
			}

			name = *srcIt;
		}


		bool matchWithFile(const std::string& filename) const
		{
			const std::size_t fileLength = filename.length();
			std::string lowerCaseName = toLower(filename);

			for(const std::string& str : extensionsLowerCase)
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
	private:
		ExtList extensionsLowerCase;
	};


	class OctExtensionsList : public std::vector<OctExtension>
	{
	public:
		OctExtensionsList() = default;
		OctExtensionsList(const std::initializer_list<OctExtension>& list) : std::vector<OctExtension>(list) {};


		bool matchWithFile(const std::string& filename) const
		{
			for(const OctExtension& ext : *this)
				if(ext.matchWithFile(filename))
					return true;
			return false;
		}
	};
}
