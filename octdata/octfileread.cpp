#include "octfileread.h"

#include <datastruct/oct.h>
#include "import/octfilereader.h"

#include <iostream>


#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;


namespace OctData
{
	OctFileRead::OctFileRead()
	{
		// extensions.push_back(OctExtension("xml","Heidelberg Engineering OCT XML"));
	}


	OctFileRead::~OctFileRead()
	{

	}


	OCT OctData::OctFileRead::openFile(const std::string& filename)
	{
		OctFileReader::registerReaders(); // TODO: serch better implementation to prevent remove by linker

		OctFileRead& obj = getInstance();
		OctData::OCT oct;


		bfs::path file(filename);

		for(OctFileReader* reader : obj.fileReaders)
		{
			if(reader->readFile(file, oct))
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


}


