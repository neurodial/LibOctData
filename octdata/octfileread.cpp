#include "octfileread.h"

#include <datastruct/oct.h>
#include "import/octfilereader.h"
#include "filereadoptions.h"

#include "buildconstants.h"

#include <boost/log/trivial.hpp>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#include "import/platform_helper.h"

namespace OctData
{
	OctFileRead::OctFileRead()
	{
		BOOST_LOG_TRIVIAL(info) << "OctData: Build Type      : " << BuildConstants::buildTyp;
		BOOST_LOG_TRIVIAL(info) << "OctData: Git Hash        : " << BuildConstants::gitSha1;
		BOOST_LOG_TRIVIAL(info) << "OctData: Build Date      : " << BuildConstants::buildDate;
		BOOST_LOG_TRIVIAL(info) << "OctData: Build Time      : " << BuildConstants::buildTime;
		BOOST_LOG_TRIVIAL(info) << "OctData: Compiler Id     : " << BuildConstants::compilerId;
		BOOST_LOG_TRIVIAL(info) << "OctData: Compiler Version: " << BuildConstants::compilerVersion;
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

		bfs::path file(filenameConv(filename));

		if(bfs::exists(file))
		{
			for(OctFileReader* reader : fileReaders)
			{
				if(reader->readFile(file, oct, op))
					break;
				oct.clear();
			}
		}
		else
			BOOST_LOG_TRIVIAL(error) << "file " << file.generic_string() << " not exists";

		return oct;
	}

// used by friend class OctFileReader
	void OctFileRead::registerFileRead(OctFileReader* reader, const OctExtension& ext)
	{
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
				if(ext == fileExt)
					return true;
		return false;
	}



}


