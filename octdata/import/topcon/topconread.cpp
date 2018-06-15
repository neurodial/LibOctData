#include "topconread.h"


#include <ostream>
#include <fstream>
#include <iomanip>
#include<array>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>


#include <cpp_framework/callback.h>

#include <datastruct/oct.h>
#include <datastruct/bscan.h>
#include <datastruct/date.h>
#include <datastruct/sloimage.h>
#include <filereadoptions.h>
#include<filereader/filereader.h>


#include "../platform_helper.h"
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

	std::string& readFStreamFull(std::istream& stream, std::string& dest, std::size_t nums)
	{
		dest.assign(nums, '\0');
		stream.read(&dest[0], nums);
		return dest;
	}

	void stripZeroCaracters(std::string& dest)
	{
		dest = std::string(dest.c_str()); // strip zero caracters
	}

	std::string& readFStream(std::istream& stream, std::string& dest, std::size_t nums)
	{
		readFStreamFull(stream, dest, nums);
		stripZeroCaracters(dest);
		return dest;
	}

	struct ReadProperty
	{
		enum class SLOImageType { Unknown, Fundus, TRC };
		SLOImageType slotype = SLOImageType::Unknown;
	};


	struct BScanPair
	{
		cv::Mat image;
		OctData::BScan::Data data;
	};

	typedef std::vector<BScanPair> BScanList;

	void readImgJpeg(std::istream& stream, OctData::Series& series, BScanList& bscanList, CppFW::Callback* callback)
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

		switch(type)
		{
			case 0:
				series.setScanPattern(OctData::Series::ScanPattern::SingleLine);
				break;
			case 2:
				series.setScanPattern(OctData::Series::ScanPattern::Volume);
				break;
			case 3:
				series.setScanPattern(OctData::Series::ScanPattern::Circular);
				break;
// 			case 7:
// 				series.setScanPattern(7 line scan (h or v));
// 				break;
// 			case 11:
// 				series.setScanPattern(2x5scans > 10 slice volume);
// 				break;
		}

		for(uint32_t frame = 0; frame < frames; ++frame)
		{
			if(callback)
			{
				if(!callback->callback(static_cast<double>(frame)/static_cast<double>(frames)))
					break;
			}
			const uint32_t size = readFStream<uint32_t>(stream);
			char* encodedData = new char[size];

			stream.read(encodedData, size);

			cv::Mat image;
			ReadJPEG2K reader;
			reader.openJpeg(encodedData, size);
			reader.getImage(image, false);

			image.convertTo(image, cv::DataType<uint8_t>::type, 2, -128);

			BScanPair pair;
			pair.image = image;
			bscanList.push_back(pair);

			delete[] encodedData;
		}
	}

	void readImgTrc(std::istream& stream, OctData::Series& series, ReadProperty& property)
	{
		if(series.getSloImage().hasImage())
			if(!series.getSloImage().getImage().empty())
				return;

		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t height = readFStream<uint32_t>(stream);
		const uint32_t bits   = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint8_t  u1     = readFStream<uint8_t >(stream);

		BOOST_LOG_TRIVIAL(debug) << "width  : " << width
		                 << '\t' << "height : " << height
		                 << '\t' << "bits   : " << bits
		                 << '\t' << "frames : " << frames
		                 << '\t' << "u1     : " << static_cast<int>(u1);

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

		property.slotype = ReadProperty::SLOImageType::TRC;

		delete[] encodedData;
	}

	void readImgFundus(std::istream& stream, OctData::Series& series, ReadProperty& property)
	{
		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t height = readFStream<uint32_t>(stream);
		const uint32_t bits   = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint32_t u1     = readFStream<uint32_t>(stream);

		BOOST_LOG_TRIVIAL(debug) << "width  : " << width
		                 << '\t' << "height : " << height
		                 << '\t' << "bits   : " << bits
		                 << '\t' << "frames : " << frames
		                 << '\t' << "u1     : " << static_cast<int>(u1);

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


		property.slotype = ReadProperty::SLOImageType::Fundus;

		delete[] encodedData;
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

	void decriptString(std::string& str, const std::array<unsigned char, 32>& key)
	{
		const std::size_t cryptLen = std::min(key.size(), str.size());
		for(std::size_t i = 0; i < cryptLen; ++i)
			str[i] = key[i]^static_cast<unsigned char>(str[i]);

		stripZeroCaracters(str);
	}

	class EncryptKeyPatientInfo03
	{
		std::array<unsigned char, 32> keyID      ;
		std::array<unsigned char, 32> keyForename;
		std::array<unsigned char, 32> keySurename;

		static void readKey(std::array<unsigned char, 32>& key, std::fstream& stream)
		{
			int c;
			for(std::size_t i = 0; i<32 && stream.good(); ++i)
			{
				stream >> c;
				key[i] = static_cast<unsigned char>(c);
			}
		}

	public:
		EncryptKeyPatientInfo03(const OctData::FileReadOptions& op)
		{
			BOOST_LOG_TRIVIAL(info) << "read topcon keys from :" << op.libPath << "/topcon_key.txt";
			std::fstream stream(op.libPath + "/topcon_key.txt");

			readKey(keyID      , stream);
			readKey(keyForename, stream);
			readKey(keySurename, stream);
		}

		const std::array<unsigned char, 32>& getKeyID      ()         { return keyID      ; }
		const std::array<unsigned char, 32>& getKeyForename()         { return keyForename; }
		const std::array<unsigned char, 32>& getKeySurename()         { return keySurename; }
	};

	void readPatientInfo03(std::istream& stream, OctData::Patient& pat, const OctData::FileReadOptions& op)
	{
		EncryptKeyPatientInfo03 keys(op);

		std::string patId;
		std::string patForeName;
		std::string patSureName;
		std::string zeros;
		readFStreamFull(stream, patId      , 32);
		readFStreamFull(stream, patForeName, 32);
		readFStreamFull(stream, patSureName, 32);
		readFStream(stream, zeros      , 8);

		readFStream<uint8_t>(stream);

		decriptString(patId      , keys.getKeyID      ());
		decriptString(patForeName, keys.getKeyForename());
		decriptString(patSureName, keys.getKeySurename());


		pat.setId       (patId      );
		pat.setForename (patForeName);
		pat.setSurname  (patSureName);
	}

	void readCaptureInfo02(std::istream& stream, OctData::Series& series)
	{
		readFStream<uint16_t>(stream);

		stream.seekg(52*sizeof(uint16_t), std::ios_base::cur);

		OctData::Date scanDate;
		scanDate.setYear (readFStream<uint16_t>(stream));
		scanDate.setMonth(readFStream<uint16_t>(stream));
		scanDate.setDay  (readFStream<uint16_t>(stream));
		scanDate.setHour (readFStream<uint16_t>(stream));
		scanDate.setMin  (readFStream<uint16_t>(stream));
		scanDate.setSec  (readFStream<uint16_t>(stream));
		scanDate.setDateAsValid();

		series.setScanDate(scanDate);
	}

	void readParamScan04(std::istream& stream, BScanList& list)
	{
		uint32_t unknown1[3];
		readFStream(stream, unknown1, sizeof(unknown1)/sizeof(unknown1[0]));
		double scanSizeXmm = readFStream<double>(stream);
		double scanSizeYmm = readFStream<double>(stream);
		double scanSizeZmm = readFStream<double>(stream);

		double resY = scanSizeYmm/static_cast<double>(list.size());
		for(BScanPair& bscan : list)
		{
			double resX = scanSizeXmm/static_cast<double>(bscan.image.cols);
			double resZ = scanSizeZmm/static_cast<double>(bscan.image.rows);
			OctData::ScaleFactor sf(resX, resY, resZ);
			bscan.data.scaleFactor = sf;
		}
	}

	void readConturInfo(std::istream& stream, BScanList& list)
	{
		class ConturInfo : public std::map<std::string, OctData::Segmentationlines::SegmentlineType>
		{
		public:
			ConturInfo() // TODO Reihenfolge anpassen
			{
				emplace("MULTILAYERS_1", OctData::Segmentationlines::SegmentlineType::ILM );
				emplace("MULTILAYERS_2", OctData::Segmentationlines::SegmentlineType::RNFL);
				emplace("MULTILAYERS_3", OctData::Segmentationlines::SegmentlineType::GCL );
				emplace("MULTILAYERS_4", OctData::Segmentationlines::SegmentlineType::IPL );
				emplace("MULTILAYERS_5", OctData::Segmentationlines::SegmentlineType::INL );
				emplace("MULTILAYERS_6", OctData::Segmentationlines::SegmentlineType::OPL );
				emplace("MULTILAYERS_7", OctData::Segmentationlines::SegmentlineType::BM  );
				emplace("MULTILAYERS_8", OctData::Segmentationlines::SegmentlineType::PR1 );
				emplace("MULTILAYERS_9", OctData::Segmentationlines::SegmentlineType::ELM );

				emplace("RETINA_1", OctData::Segmentationlines::SegmentlineType::ILM );
				emplace("RETINA_2", OctData::Segmentationlines::SegmentlineType::RNFL);
				emplace("RETINA_3", OctData::Segmentationlines::SegmentlineType::INL );
				emplace("RETINA_4", OctData::Segmentationlines::SegmentlineType::BM  );

				emplace("NFL_1", OctData::Segmentationlines::SegmentlineType::GCL );
				emplace("NFL_2", OctData::Segmentationlines::SegmentlineType::IPL );
				emplace("NFL_3", OctData::Segmentationlines::SegmentlineType::OPL );
			}
		};

		static ConturInfo conturInfo;

		std::string id;
		readFStream(stream, id, 20);
		const uint16_t type   = readFStream<uint16_t>(stream);
		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		const uint32_t size   = readFStream<uint32_t>(stream);


		const ConturInfo::const_iterator contId = conturInfo.find(id);
		if(contId == conturInfo.end())
		{
			BOOST_LOG_TRIVIAL(error) << "unhandled id: " << id    ;
			return; // TODO: unhandled
		}
		const OctData::Segmentationlines::SegmentlineType lineType =contId->second;


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

					OctData::Segmentationlines::Segmentline line;
					readFStream(stream, tmpVec, width);
					for(uint32_t ascan = 0; ascan < width; ++ascan)
						line.push_back(imgHeight - static_cast<OctData::Segmentationlines::SegmentlineDataType>(tmpVec[ascan]));
					bscanPair.data.getSegmentLine(lineType) = std::move(line);
				}
			}
			delete[] tmpVec;
		}
	}

	void readRegistInfo(std::istream& stream, BScanList& list, ReadProperty& property)
	{
		uint32_t unknown1   [ 2];
		uint32_t boundFundus[ 4];
		uint8_t  unknown2   [32];
		uint32_t boundTrc   [ 4];

		readFStream<uint8_t> (stream);
		readFStream(stream, unknown1   , sizeof(unknown1   )/sizeof(unknown1   [0]));
		readFStream(stream, boundFundus, sizeof(boundFundus)/sizeof(boundFundus[0]));
		readFStream(stream, unknown2   , sizeof(unknown2   )/sizeof(unknown2   [0]));
		readFStream(stream, boundTrc   , sizeof(boundTrc   )/sizeof(boundTrc   [0]));

		const std::size_t numBScans = list.size();

		std::size_t bscanNum = 0;
		for(BScanPair& bscan : list)
		{
			double pos = static_cast<double>(bscanNum)/static_cast<double>(numBScans);

			switch(property.slotype)
			{
				case ReadProperty::SLOImageType::Unknown:
					break;
				case ReadProperty::SLOImageType::Fundus:
					bscan.data.start = OctData::CoordSLOmm(boundFundus[0]*pos + boundFundus[2]*(1.-pos), boundFundus[1]);
					bscan.data.end   = OctData::CoordSLOmm(boundFundus[0]*pos + boundFundus[2]*(1.-pos), boundFundus[3]);
					break;
				case ReadProperty::SLOImageType::TRC:
					bscan.data.start = OctData::CoordSLOmm(boundTrc[0]*pos + boundTrc[2]*(1.-pos), boundTrc[1]);
					bscan.data.end   = OctData::CoordSLOmm(boundTrc[0]*pos + boundTrc[2]*(1.-pos), boundTrc[3]);
					break;
			}

			++bscanNum;
		}
	}

}




namespace OctData
{

	TopconFileFormatRead::TopconFileFormatRead()
	: OctFileReader(OctExtension(".fda", "Topcon"))
	{
	}

	bool TopconFileFormatRead::readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();
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


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1);

		BScanList bscanList;

		ReadProperty property;

		uint8_t chunkNameSize;
		while(readFStream(stream, chunkNameSize) > 0)
		{
			std::string chunkName;
			readFStream(stream, chunkName, chunkNameSize);
			if(chunkName[0] != '@')
			{
				BOOST_LOG_TRIVIAL(debug) << "Break: chunkName "<< " (@" << stream.tellg() << ")";
				break;
			}
			uint32_t chunkSize;
			readFStream(stream, chunkSize);

			const std::size_t chunkBegin  = stream.tellg();
			BOOST_LOG_TRIVIAL(debug) << "chunkName "<< " (@" << chunkBegin << " -> " << static_cast<int>(chunkSize) << ") : " << chunkName ;

			if(chunkName == "@IMG_TRC_02")
				readImgTrc(stream, series, property);
			else if(chunkName == "@IMG_JPEG")
				readImgJpeg(stream, series, bscanList, callback);
			else if(chunkName == "@PATIENT_INFO_02")
				readPatientInfo02(stream, pat);
			else if(chunkName == "@PATIENT_INFO_03")
				readPatientInfo03(stream, pat, op);
			else if(chunkName == "@CONTOUR_INFO")
				readConturInfo(stream, bscanList);
			else if(chunkName == "@CAPTURE_INFO_02")
				readCaptureInfo02(stream, series);
			else if(chunkName == "@IMG_FUNDUS")
				readImgFundus(stream, series, property);
			else if(chunkName == "@REGIST_INFO")
				readRegistInfo(stream, bscanList, property);
			else if(chunkName == "@PARAM_SCAN_04")
				readParamScan04(stream, bscanList);

			stream.seekg(chunkBegin + chunkSize);
		}

		for(BScanPair& pair : bscanList)
		{
			OctData::BScan::Data bscanData;
			OctData::BScan* bscan = new OctData::BScan(pair.image, pair.data);

			series.takeBScan(bscan);
		}


		BOOST_LOG_TRIVIAL(debug) << "read oct file \"" << file.generic_string() << "\" finished";
		return true;
	}

}
