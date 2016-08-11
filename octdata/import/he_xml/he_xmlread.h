#pragma once
#include <string>

#include "../octfilereader.h"


#include <boost/filesystem.hpp>

class SLOImage;
class CScan;
class AlgoBatch;

namespace OctData
{

	class HeXmlRead : public OctFileReader
	{
		HeXmlRead();

		boost::filesystem::path xmlFilename;
		boost::filesystem::path xmlPath;


	public:

		static HeXmlRead* getInstance();

		virtual bool readFile(const boost::filesystem::path& file, OCT& oct);
	};

}