#include "giplread.h"

#include<boost/endian/arithmetic.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

namespace bfs = boost::filesystem;

#include <opencv2/opencv.hpp>

#include <cpp_framework/callback.h>

#include "../platform_helper.h"
#include "../../octdata_packhelper.h"
#include <datastruct/oct.h>
#include <datastruct/patient.h>
#include <datastruct/series.h>
#include <datastruct/bscan.h>

#include <filereadoptions.h>


#include<filereader/filereader.h>


// GIPL magic number
const unsigned GIPL_MAGIC1 = 719555000;
const unsigned GIPL_MAGIC2 = 4026526128U;

// GIPL header size
#define GIPL_HEADERSIZE 256

// GIPL filter types
#define GIPL_BINARY       1
#define GIPL_CHAR         7
#define GIPL_U_CHAR       8
#define GIPL_U_SHORT     15
#define GIPL_SHORT       16
#define GIPL_U_INT       31
#define GIPL_INT         32
#define GIPL_FLOAT       64
#define GIPL_DOUBLE      65
#define GIPL_C_SHORT    144
#define GIPL_C_INT      160
#define GIPL_C_FLOAT    192
#define GIPL_C_DOUBLE   193


namespace
{
	template<typename T>
	void readFStream(std::istream& stream, T* dest, std::size_t num = 1)
	{
		stream.read(reinterpret_cast<char*>(dest), sizeof(T)*num);
	}

	template<typename T>
	void readCVImage(std::istream& stream, cv::Mat& image, std::size_t sizeX, std::size_t sizeY)
	{
		image = cv::Mat(static_cast<int>(sizeX), static_cast<int>(sizeY), cv::DataType<T>::type);

		std::size_t num = sizeX*sizeY;

		stream.read(reinterpret_cast<char*>(image.data), num*sizeof(T));
	}

	struct GiplHeader
	{
		PACKSTRUCT(struct RawData
		{
			uint16_t sizeX           ;
			uint16_t sizeY           ;
			uint16_t sizeZ           ;
			uint16_t n            ;
			uint16_t type            ;
			float    scaleX          ;
			float    scaleY          ;
			float    scaleZ          ;
		});
		RawData data;

		uint16_t getSizeX () const                                     { return boost::endian::big_to_native(data.sizeX ); }
		uint16_t getSizeY () const                                     { return boost::endian::big_to_native(data.sizeY ); }
		uint16_t getSizeZ () const                                     { return boost::endian::big_to_native(data.sizeZ ); }
		uint16_t getType  () const                                     { return boost::endian::big_to_native(data.type  ); }
// 		float    getScaleX() const                                     { return boost::endian::big_to_native(data.scaleX); }
// 		float    getScaleY() const                                     { return boost::endian::big_to_native(data.scaleY); }
// 		float    getScaleZ() const                                     { return boost::endian::big_to_native(data.scaleZ); }
	};
}


namespace OctData
{

	bool OctData::GIPLRead::readFile(FileReader& filereader, OctData::OCT& oct, const OctData::FileReadOptions& op, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();

		if(file.extension() != ".gipl")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as gpil";


		std::fstream stream(filepathConv(file), std::ios::binary | std::ios::in);
		if(!stream.good())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open gpil file " << filepathConv(file);
			return false;
		}

		BOOST_LOG_TRIVIAL(debug) << "open " << file.generic_string() << " as gpil file";

		std::string dir      = file.branch_path().generic_string();
		std::string filename = file.filename().generic_string();


		GiplHeader volHeader;

		readFStream(stream, &(volHeader.data));

		if(volHeader.getType() != GIPL_U_CHAR)
		{
			BOOST_LOG_TRIVIAL(error) << "Non supported format (only uint8 supported) " << volHeader.getType();
			return false;
		}


		stream.seekg(GIPL_HEADERSIZE);


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1); // TODO

		const std::size_t sizeX     = volHeader.getSizeX();
		const std::size_t sizeY     = volHeader.getSizeY();
		const std::size_t numBScans = volHeader.getSizeZ();
		// Read BScann
		for(std::size_t numBscan = 0; numBscan<numBScans; ++numBscan)
		{
			if(callback)
				callback->callback(static_cast<double>(numBscan)/static_cast<double>(numBScans));

			cv::Mat bscanImage;
			readCVImage<uint8_t>(stream, bscanImage, sizeY, sizeX);


			BScan::Data bscanData;
			BScan* bscan = new BScan(bscanImage, bscanData);
			if(op.holdRawData)
				bscan->setRawImage(bscanImage);
			series.takeBScan(bscan);
		}

		return true;
	}


	OctData::GIPLRead::GIPLRead()
	: OctFileReader(OctExtension(".gipl", "Guys Image Processing Lab Format"))
	{
	}


}
