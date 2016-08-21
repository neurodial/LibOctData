#include "cirrus_rawread.h"

#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>


namespace bfs = boost::filesystem;


namespace
{
	void readCVImage(std::istream& stream, cv::Mat& image, int cvFormat, int factor, std::size_t sizeX, std::size_t sizeY)
	{
		image = cv::Mat(static_cast<int>(sizeX), static_cast<int>(sizeY), cvFormat);

		std::size_t num = sizeX*sizeY;

		stream.read(reinterpret_cast<char*>(image.data), num*factor);
	}
}


namespace OctData
{
	CirrusRawRead::CirrusRawRead()
	: OctFileReader(OctExtension(".img", "Cirrus img files"))
	{

	}

	bool CirrusRawRead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& /*op*/)
	{
		if(file.extension() != ".img")
			return false;

		if(!bfs::exists(file))
			return false;

		bool debug = false;

		// split filename in elements
		std::vector<std::string> elements;
		std::string filenameString = file.filename().generic_string();
		boost::split(elements, filenameString, boost::is_any_of("_"), boost::token_compress_on);

		if(debug)
			std::cout << "elements.size(): " << elements.size() << std::endl;

		if(elements.size() != 8)
			return false;

		const std::string& patient_id = elements.at(0);
		const std::string& scantype   = elements.at(1);
		const std::string& scan_date1 = elements.at(2);
		const std::string& scan_date2 = elements.at(3);
		const std::string& eye_side   = elements.at(4);
		const std::string& sn         = elements.at(5);
		const std::string& cube       = elements.at(6);
		const std::string& filetype   = elements.at(7);

		if(debug)
		{
			std::cout << "patient_id: " << patient_id << std::endl;
			std::cout << "scantype  : " << scantype   << std::endl;
			std::cout << "scan_date1: " << scan_date1 << std::endl;
			std::cout << "scan_date2: " << scan_date2 << std::endl;
			std::cout << "eye_side  : " << eye_side   << std::endl;
			std::cout << "sn        : " << sn         << std::endl;
			std::cout << "cube      : " << cube       << std::endl;
			std::cout << "filetype  : " << filetype   << std::endl;
		}

		if(filetype != "raw.img" && filetype != "z.img")
			return false;


		// split scantype in elements to find the scan size
		std::vector<std::string> scantypeElements;
		boost::split(scantypeElements, scantype, boost::is_any_of(" "), boost::token_compress_on);

		if(debug)
			std::cout << "scantypeElements.size(): " << scantypeElements.size() << std::endl;
		if(scantypeElements.size() != 4)
			return false;

		std::vector<std::string> scanSizeElements;
 		const std::string& vol_size   = scantypeElements.at(3);
		boost::split(scanSizeElements, vol_size, boost::is_any_of("x"), boost::token_compress_on);
		if(scanSizeElements.size() != 2)
			return false;

		std::size_t volSizeX = boost::lexical_cast<std::size_t>(scanSizeElements[0]);
		std::size_t volSizeY = boost::lexical_cast<std::size_t>(scanSizeElements[1]);

		if(debug)
			std::cout << "vol_size " << volSizeX << " : " << volSizeY << std::endl;

		// TODO:
/*
if strcmp(cube,'hidef')
    num_bscan = 2;
    if strcmp(vol_info.scan_type,'Macular')
        num_ascan = 1024;
    elseif strcmp(vol_info.scan_type,'Optic Disc')
        num_ascan = 1000;
    end
end
*/

		std::size_t filesize = bfs::file_size(file);
		std::size_t volSizeZ = filesize / volSizeX /volSizeY;

		std::fstream stream(file.generic_string(), std::ios::binary | std::ios::in);
		if(!stream.good())
			return false;

		Patient& pat    = oct.getPatient(0);
		Series&  series = pat.getStudy(0).getSeries(0);

		for(std::size_t i = 0; i<volSizeY; ++i)
		{
			cv::Mat bscanImage;
			int sloCvFormat = CV_8UC1;
			int sloFactor   = 1;
			readCVImage(stream, bscanImage, sloCvFormat, sloFactor, volSizeZ, volSizeX);

			cv::flip(bscanImage, bscanImage, 0);

			BScan* bscan = new BScan(bscanImage, BScan::Data());
			series.takeBScan(bscan);
		}

		//------------
		// load slo
		//------------
		std::string fileString = file.generic_string();
		std::size_t found = fileString.find_last_of("_");
		found = fileString.find_last_of("_", found-1);
		std::string baseFilename = fileString.substr(0, found);

		bfs::path slofile(baseFilename + "_lslo.bin");
		std::cout << slofile.generic_string() << std::endl;
		if(!bfs::exists(slofile))
			return true; // bscans loaded successfull


		std::fstream streamSlo(slofile.generic_string(), std::ios::binary | std::ios::in);
		if(!streamSlo.good())
			return true; // bscans loaded successfull

		cv::Mat sloImage;
		int sloCvFormat = CV_8UC1;
		int sloFactor   = 1;

		std::size_t filesizeSlo = bfs::file_size(slofile);

		std::size_t sloWidth = 512;
		readCVImage(streamSlo, sloImage, sloCvFormat, sloFactor, sloWidth, filesizeSlo/sloWidth);
		SloImage* slo = new SloImage;
		slo->setImage(sloImage);
		series.takeSloImage(slo);

		if(debug)
			std::cout << "slo: " << sloWidth << " x " << (filesizeSlo/sloWidth) << std::endl;
		return true;
	}


	CirrusRawRead* CirrusRawRead::getInstance()
	{
		static CirrusRawRead instance; return &instance;
	}


}
