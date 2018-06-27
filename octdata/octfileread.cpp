#include "octfileread.h"

#include <datastruct/oct.h>
#include "import/octfilereader.h"
#include "filereadoptions.h"
#include "filewriteoptions.h"

#include<opencv/cv.hpp>

#include "buildconstants.h"

#include <boost/log/trivial.hpp>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#include<filereader/filereader.h>

#include "import/platform_helper.h"
#include<export/cirrus_raw/cirrusrawexport.h>
#include<export/xoct/xoctwrite.h>
#include<export/cvbin/cvbinoctwrite.h>

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
		BOOST_LOG_TRIVIAL(info) << "OctData: OpenCV Version  : " << CV_VERSION ; // cv::getBuildInformation();

		OctFileReader::registerReaders(*this); // TODO: serch better implementation
	}


	OctFileRead::~OctFileRead()
	{
		for(OctFileReader* reader : fileReaders)
			delete reader;
	}


	OCT OctFileRead::openFile(const std::string& filename, CppFW::Callback* callback)
	{
		return getInstance().openFilePrivat(filename, FileReadOptions(), callback);
	}

	OCT OctFileRead::openFile(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback)
	{
		return getInstance().openFilePrivat(filename, op, callback);
	}

	OCT OctFileRead::openFile(const boost::filesystem::path& filename, const FileReadOptions& op, CppFW::Callback* callback)
	{
		return getInstance().openFilePrivat(filename, op, callback);
	}



	OCT OctFileRead::openFilePrivat(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback)
	{
		bfs::path file(filenameConv(filename));
		return openFilePrivat(file, op, callback);
	}



	bool OctFileRead::openFileFromExt(OCT& oct, FileReader& filereader, const FileReadOptions& op, CppFW::Callback* callback)
	{
		std::string filename = filereader.getFilepath().generic_string();
		for(OctFileReader* reader : fileReaders)
		{
			if(reader->getExtentsions().matchWithFile(filename))
			{
				if(reader->readFile(filereader, oct, op, callback))
					return true;
				oct.clear();
			}
		}
		return false;
	}

	bool OctFileRead::tryOpenFile(OCT& oct, FileReader& filereader, const FileReadOptions& op, CppFW::Callback* callback)
	{
		for(OctFileReader* reader : fileReaders)
		{
			if(reader->readFile(filereader, oct, op, callback))
				return true;
			oct.clear();
		}
		return false;
	}

	OCT OctFileRead::openFilePrivat(const boost::filesystem::path& file, const FileReadOptions& op, CppFW::Callback* callback)
	{
		FileReader filereader(file);
		OctData::OCT oct;

		if(bfs::exists(file))
		{
			if(!openFileFromExt(oct, filereader, op, callback))
				tryOpenFile(oct, filereader, op, callback);
		}
		else
			BOOST_LOG_TRIVIAL(error) << "file " << file.generic_string() << " not exists";

		return oct;
	}

// used by friend class OctFileReader
	void OctFileRead::registerFileRead(OctFileReader* reader)
	{
		if(!reader)
			return;

		const OctExtensionsList& extList = reader->getExtentsions();

		for(const OctExtension& ext : extList)
		{
			BOOST_LOG_TRIVIAL(info) << "OctData: register reader for " << ext.name;
			extensions.push_back(ext);
		}

		fileReaders.push_back(reader);
	}

	const OctExtensionsList& OctFileRead::supportedExtensions()
	{
		return getInstance().extensions;
	}

	bool OctFileRead::isLoadable(const std::string& filename)
	{

		for(const OctExtension& supportedExtension : getInstance().extensions)
			if(supportedExtension.matchWithFile(filename))
				return true;
		return false;
	}


	bool OctFileRead::writeFile(const std::string& filename, const OCT& octdata)
	{
		return getInstance().writeFilePrivat(filename, octdata, FileWriteOptions());
	}

	bool OctFileRead::writeFile(const std::string& filename, const OCT& octdata, const FileWriteOptions& opt)
	{
		return getInstance().writeFilePrivat(filename, octdata, opt);
	}

	bool OctFileRead::writeFile(const bfs::path& filepath, const OCT& octdata, const FileWriteOptions& opt)
	{
		return getInstance().writeFilePrivat(filepath, octdata, opt);
	}


	bool OctFileRead::writeFilePrivat(const bfs::path& filepath, const OCT& octdata, const FileWriteOptions& opt)
	{
		if(filepath.extension() == ".img")
			return CirrusRawExport::writeFile(filepath, octdata, opt);
		if(filepath.extension() == ".xoct")
			return XOctWrite::writeFile(filepath, octdata, opt);
		return CvBinOctWrite::writeFile(filepath, octdata, opt);
	}

}


