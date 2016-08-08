#pragma once

#include <vector>
#include <string>

namespace OctData
{
	class OCT;

	struct OctExtension
	{
		OctExtension(const std::string& ext, const std::string& name)
		: extension(ext)
		, name(name)
		{}

		std::string extension;
		std::string name;
	};

	class OctFileRead
	{
	public:
		typedef std::vector<OctExtension> ExtensionsList;
		static OctFileRead& getInstance()                        { static OctFileRead instance; return instance; }

		static const ExtensionsList& supportedExtensions()       { return getInstance().extensions; };
		static OCT&& openFile(const std::string& filename);

	private:
		OctFileRead();
		~OctFileRead();

		ExtensionsList extensions;
	};
	
}

