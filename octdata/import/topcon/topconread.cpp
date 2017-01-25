#include "topconread.h"

#include <datastruct/oct.h>
#include <datastruct/bscan.h>
#include <datastruct/date.h>
#include <datastruct/sloimage.h>

#include <ostream>
#include <fstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include "../platform_helper.h"
#include <filereadoptions.h>


#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include "readjpeg2k.h"


namespace bfs = boost::filesystem;


namespace
{
	template<typename T>
	void readFStream(std::istream& stream, T* dest, std::size_t num = 1)
	{
		stream.read(reinterpret_cast<char*>(dest), sizeof(T)*num);
	}

	template<typename T>
	T& readFStream(std::istream& stream, T& dest)
	{
		readFStream(stream, &dest, 1);
		return dest;
	}


	template<typename T>
	T readFStream(std::istream& stream)
	{
		T dest;
		readFStream(stream, &dest, 1);
		return dest;
	}

	std::string& readFStream(std::istream& stream, std::string& dest, std::size_t nums)
	{
		dest.assign(nums, '\0');
		stream.read(&dest[0], nums);
		return dest;
	}


	struct BScanPair
	{
		cv::Mat image;
		OctData::BScan::Data data;
	};

	typedef std::vector<BScanPair> BScanList;

	void readImgJpeg(std::istream& stream, BScanList& bscanList)
	{
		const uint8_t  type   = readFStream<uint8_t >(stream);
		const uint32_t u1     = readFStream<uint32_t>(stream);
		const uint32_t u2     = readFStream<uint32_t>(stream);
		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t height = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint32_t u3     = readFStream<uint32_t>(stream);

		BOOST_LOG_TRIVIAL(debug) << "type  : " << static_cast<int>(type)  ;
		BOOST_LOG_TRIVIAL(debug) << "u1    : " << u1    ;
		BOOST_LOG_TRIVIAL(debug) << "u2    : " << u2    ;
		BOOST_LOG_TRIVIAL(debug) << "width : " << width ;
		BOOST_LOG_TRIVIAL(debug) << "height: " << height;
		BOOST_LOG_TRIVIAL(debug) << "frames: " << frames;
		BOOST_LOG_TRIVIAL(debug) << "u3    : " << u3    ;

		for(uint32_t frame = 0; frame < frames; ++frame)
		{
			const uint32_t size = readFStream<uint32_t>(stream);
			char* encodedData = new char[size];

			stream.read(encodedData, size);

			cv::Mat image;
			ReadJPEG2K reader;
			reader.openJpeg(encodedData, size);
			reader.getImage(image, false);

			BScanPair pair;
			pair.image = image;
			bscanList.push_back(pair);

			delete[] encodedData;
		}
	}

	void readImgTrc(std::istream& stream, OctData::Series& series)
	{
		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t height = readFStream<uint32_t>(stream);
		const uint32_t bits   = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint8_t  u1     = readFStream<uint8_t >(stream);

		BOOST_LOG_TRIVIAL(debug) << "width  : " << width ;
		BOOST_LOG_TRIVIAL(debug) << "height : " << height;
		BOOST_LOG_TRIVIAL(debug) << "bits   : " << bits  ;
		BOOST_LOG_TRIVIAL(debug) << "frames : " << frames;
		BOOST_LOG_TRIVIAL(debug) << "u1     : " << static_cast<int>(u1)    ;

		// use only the first image

		const uint32_t size = readFStream<uint32_t>(stream);
		char* encodedData = new char[size];

		stream.read(encodedData, size);

		cv::Mat image;
		ReadJPEG2K reader;
		reader.openJpeg(encodedData, size);
		reader.getImage(image, false);

		OctData::SloImage* sloImage = new OctData::SloImage;
		sloImage->setImage(image);
		series.takeSloImage(sloImage);
	}

	void readPatientInfo02(std::istream& stream, OctData::Patient& pat)
	{
		std::string patId;
		std::string patForeName;
		std::string patSureName;
		std::string zeros;
		readFStream(stream, patId      , 32);
		readFStream(stream, patForeName, 32);
		readFStream(stream, patSureName, 32);
		readFStream(stream, zeros      , 8);

		readFStream<uint8_t>(stream);

		OctData::Date birthDate;
		birthDate.setYear (readFStream<uint16_t>(stream));
		birthDate.setMonth(readFStream<uint16_t>(stream));
		birthDate.setDay  (readFStream<uint16_t>(stream));
		birthDate.setDateAsValid();

		pat.setId       (patId      );
		pat.setForename (patForeName);
		pat.setSurname  (patSureName);
		pat.setBirthdate(birthDate  );
	}

	bool cmpString(const char* strA, const std::string& strB)
	{
		std::size_t strLenA = strlen(strA);
		std::size_t strLenB = strB.size();
		if(strLenA > strLenB)
			return false;

		return memcmp(strA, strB.c_str(), strLenA) == 0;
	}

	void readConturInfo(std::istream& stream, BScanList& list)
	{
		std::string id;
		readFStream(stream, id, 20);
		const uint16_t type   = readFStream<uint16_t>(stream);
		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint32_t size   = readFStream<uint32_t>(stream);

		OctData::BScan::SegmentlineType lineType = OctData::BScan::SegmentlineType::I16T1;


// 		enum class SegmentlineType{ ILM, NFL, I3T1, I4T1, I5T1, I6T1, I8T3, I14T1, I15T1, I16T1, BM, NR_OF_ELEMENTS };
		if(cmpString("MULTILAYERS_1", id))
			lineType = OctData::BScan::SegmentlineType::ILM;
		else if(cmpString("MULTILAYERS_2", id))
			lineType = OctData::BScan::SegmentlineType::NFL;
		else if(cmpString("MULTILAYERS_3", id))
			lineType = OctData::BScan::SegmentlineType::I3T1;
		else if(cmpString("MULTILAYERS_4", id))
			lineType = OctData::BScan::SegmentlineType::I4T1;
		else if(cmpString("MULTILAYERS_5", id))
			lineType = OctData::BScan::SegmentlineType::I5T1;
		else if(cmpString("MULTILAYERS_6", id))
			lineType = OctData::BScan::SegmentlineType::I6T1;
		else if(cmpString("MULTILAYERS_7", id))
			lineType = OctData::BScan::SegmentlineType::I8T3;
		else if(cmpString("MULTILAYERS_8", id))
			lineType = OctData::BScan::SegmentlineType::I14T1;
		else if(cmpString("MULTILAYERS_9", id))
			lineType = OctData::BScan::SegmentlineType::BM;
		else
		{
			BOOST_LOG_TRIVIAL(error) << "unhandled id: " << id    ;
			return; // TODO: unhandled
		}


		BOOST_LOG_TRIVIAL(debug) << "id     : " << id    ;
		BOOST_LOG_TRIVIAL(debug) << "type   : " << type  ;
		BOOST_LOG_TRIVIAL(debug) << "width  : " << width ;
		BOOST_LOG_TRIVIAL(debug) << "frames : " << frames;
		BOOST_LOG_TRIVIAL(debug) << "size   : " << size  ;
		if(type == 0)
		{
			uint16_t* tmpVec = new uint16_t[width];
			for(uint32_t frame = 0; frame < frames; ++frame)
			{
				uint32_t actFrame = frames-frame-1;
				if(actFrame < list.size())
				{
					BScanPair& bscanPair = list.at(actFrame);
					int imgHeight = bscanPair.image.rows;

					OctData::BScan::Segmentline line;
					readFStream(stream, tmpVec, width);
					for(uint32_t ascan = 0; ascan < width; ++ascan)
					{
// 						std::cout << tmpVec[ascan] << std::endl;
						line.push_back(imgHeight - static_cast<OctData::BScan::SegmentlineDataType>(tmpVec[ascan]));
					}
					bscanPair.data.getSegmentLine(lineType) = line;
				}
			}
			delete[] tmpVec;

		}
	}

}




namespace OctData
{

	TopconFileFormatRead::TopconFileFormatRead()
	: OctFileReader(OctExtension(".fda", "Topcon"))
	{
	}

	bool TopconFileFormatRead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& /*op*/)
	{
//
//     BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
//     BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
//     BOOST_LOG_TRIVIAL(info) << "An informational severity message";
//     BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
//     BOOST_LOG_TRIVIAL(error) << "An error severity message";
//     BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

		if(file.extension() != ".fda")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as topcon file";


		std::fstream stream(filepathConv(file), std::ios::binary | std::ios::in);
		if(!stream.good())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open topcon file " << filepathConv(file);
			return false;
		}

		BOOST_LOG_TRIVIAL(debug) << "open " << file.generic_string() << " as topcon file";

		std::string dir      = file.branch_path().generic_string();
		std::string filename = file.filename   ().generic_string();

		const unsigned char magicHeader[] = { 'F', 'O', 'C', 'T' };


		const std::size_t formatstringlength = sizeof(magicHeader)/sizeof(magicHeader[0]);
		char fileformatstring[formatstringlength];
		readFStream(stream, fileformatstring, formatstringlength);
		if(memcmp(fileformatstring, magicHeader, formatstringlength) != 0) // 0 = strings are equal
		{
			BOOST_LOG_TRIVIAL(error) << file.generic_string() << " Wrong fileformat (wrong header)";
			return false;
		}

		char type[4];
		readFStream(stream, type, 3);
		type[3] = '\0';

		uint32_t version1;
		uint32_t version2;

		readFStream(stream, version1);
		readFStream(stream, version2);

		uint8_t chunkNameSize;


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1);

		BScanList bscanList;


		while(readFStream(stream, chunkNameSize) > 0)
		{
			std::string chunkName;
			readFStream(stream, chunkName, chunkNameSize);
			if(chunkName[0] != '@')
				break;
			uint32_t chunkSize;
			readFStream(stream, chunkSize);

			const std::size_t chunkBegin  = stream.tellg();
			BOOST_LOG_TRIVIAL(debug) << "chunkName "<< " (@" << chunkBegin << " -> " << static_cast<int>(chunkSize) << ") : " << chunkName ;

			if(chunkName == "@IMG_TRC_02")
				readImgTrc(stream, series);
			else if(chunkName == "@IMG_JPEG")
				readImgJpeg(stream, bscanList);
			else if(chunkName == "@PATIENT_INFO_02")
				readPatientInfo02(stream, pat);
			else if(chunkName == "@CONTOUR_INFO")
				readConturInfo(stream, bscanList);

			stream.seekg(chunkBegin + chunkSize);
		}

		for(BScanPair& pair : bscanList)
		{
			OctData::BScan::Data bscanData;
			OctData::BScan* bscan = new OctData::BScan(pair.image, pair.data);

			series.takeBScan(bscan);
		}


// 		Patient& pat    = oct.getPatient(1);
// 		Study&   study  = pat.getStudy(1);
// 		Series&  series = study.getSeries(1);
//
//
//
// 		MainDict mainDict(series, op);
//
// 		stream.seekg(0, std::ios_base::end);
// 		std::size_t fielsize = stream.tellg();
// 		stream.seekg(6, std::ios_base::beg);
//
// 		readDict(stream, mainDict, fielsize);
//


		BOOST_LOG_TRIVIAL(debug) << "read oct file \"" << file.generic_string() << "\" finished";
		return true;
	}

	TopconFileFormatRead* TopconFileFormatRead::getInstance()
	{
		static TopconFileFormatRead instance;
		return &instance;
	}


}
