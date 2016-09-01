#include "octfileread.h"

#include <datastruct/oct.h>
#include "import/octfilereader.h"
#include "filereadoptions.h"

#include <iostream>


#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;


namespace OctData
{
	OctFileRead::OctFileRead()
	{
	}


	OctFileRead::~OctFileRead()
	{

	}


	OCT OctFileRead::openFile(const std::string& filename)
	{
		return getInstance().openFilePrivat(filename, FileReadOptions());
	}

	OCT OctFileRead::openFile(const std::string& filename, const FileReadOptions& op)
	{
		return getInstance().openFilePrivat(filename, op);
	}



	OCT OctFileRead::openFilePrivat(const std::string& filename, const FileReadOptions& op)
	{
		OctFileReader::registerReaders(); // TODO: serch better implementation to prevent remove by linker

		OctData::OCT oct;

		bfs::path file(filename);

		for(OctFileReader* reader : fileReaders)
		{
			if(reader->readFile(file, oct, op))
				break;
			oct.clear();
		}

		return std::move(oct);
	}


	void OctFileRead::registerFileRead(OctFileReader* reader, const OctExtension& ext)
	{
		std::cout << "register: " << ext.name << std::endl;
		extensions.push_back(ext);
		fileReaders.push_back(reader);
	}

	const OctFileRead::ExtensionsList& OctFileRead::supportedExtensions()
	{
		OctFileReader::registerReaders(); // TODO: serch better implementation to prevent remove by linker
		
		return getInstance().extensions;
	}

	bool OctFileRead::isLoadable(const std::string& filename)
	{
		const bfs::path file(filename);
		const std::string fileExt = file.extension().generic_string();

		for(const OctExtension& supportedExtension : getInstance().extensions)
			for(const std::string& ext : supportedExtension.extensions)
				// std::cout << ext << " - " << fileExt << std::endl;
				if(ext == fileExt)
					return true;
		return false;
	}



}


