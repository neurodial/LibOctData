#include "topconread.h"

#define _USE_MATH_DEFINES
#include<ostream>
#include<fstream>
#include<iomanip>
#include<array>
#include<cmath>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>


#include <oct_cpp_framework/callback.h>

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

	cv::Mat readAndEncodeJPEG2kData(std::istream& stream, uint32_t size)
	{
		std::unique_ptr<char> encodedData(new char[size]);
		stream.read(encodedData.get(), size);

		cv::Mat image;
		ReadJPEG2K reader;
		reader.openJpeg(encodedData.get(), size);
		reader.getImage(image, false);

		return image;
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

	struct ScanParameter
	{
		double scanSizeXmm = 0;
		double scanSizeYmm = 0;
		double resZum      = 0;
	};

	typedef std::vector<BScanPair> BScanList;

	void readImgJpeg(std::istream& stream, OctData::Series& series, BScanList& bscanList, CppFW::Callback* callback, const OctData::FileReadOptions& op)
	{
		if(!op.readBScans)
			return;

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

		OctData::BScan::BScanType bscanType = OctData::BScan::BScanType::Line;

		switch(type)
		{
			case 0:
				series.setScanPattern(OctData::Series::ScanPattern::SingleLine);
				break;
			case 1:
				series.setScanPattern(OctData::Series::ScanPattern::Circular);
				bscanType = OctData::BScan::BScanType::Circle;
				break;
			case 2:
				series.setScanPattern(OctData::Series::ScanPattern::Volume);
				break;
// 			case 3:
// 				series.setScanPattern(OctData::Series::ScanPattern::Circular);
// 				break;
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
			cv::Mat image = readAndEncodeJPEG2kData(stream, size);

			image.convertTo(image, cv::DataType<uint8_t>::type, 2, -128);

			BScanPair pair;
			pair.image = image;
			pair.data.bscanType = bscanType;
			bscanList.push_back(pair);

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
		cv::Mat image = readAndEncodeJPEG2kData(stream, size);

		OctData::SloImage* sloImage = new OctData::SloImage;
		sloImage->setImage(image);
		series.takeSloImage(sloImage);

		property.slotype = ReadProperty::SLOImageType::TRC;

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

		cv::Mat image = readAndEncodeJPEG2kData(stream, size);

		OctData::SloImage* sloImage = new OctData::SloImage;
		sloImage->setImage(image);
		series.takeSloImage(sloImage);

		property.slotype = ReadProperty::SLOImageType::Fundus;
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

		bool keyLoaded = false;

	public:
		EncryptKeyPatientInfo03(const OctData::FileReadOptions& op)
		{
			BOOST_LOG_TRIVIAL(info) << "read topcon keys from :" << op.libPath << "/topcon_key.txt";
			std::fstream stream(op.libPath + "/topcon_key.txt");
			keyLoaded = stream.good();

			readKey(keyID      , stream);
			readKey(keyForename, stream);
			readKey(keySurename, stream);
		}

		const std::array<unsigned char, 32>& getKeyID      ()          { return keyID      ; }
		const std::array<unsigned char, 32>& getKeyForename()          { return keyForename; }
		const std::array<unsigned char, 32>& getKeySurename()          { return keySurename; }

		bool isKeyLoaded()                                       const { return keyLoaded; }
	};

	void readPatientInfo03(std::istream& stream, OctData::Patient& pat, const OctData::FileReadOptions& op)
	{
		EncryptKeyPatientInfo03 keys(op);

		if(!keys.isKeyLoaded())
			return;

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
		uint8_t lateralityByte = readFStream<uint8_t>(stream);
		/*uint8_t unknownByte */ readFStream<uint8_t>(stream);


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

		switch(lateralityByte)
		{
			case 0: series.setLaterality(OctData::Series::Laterality::OD); break;
			case 1: series.setLaterality(OctData::Series::Laterality::OS); break;
		}
	}

	void readParamScan04(std::istream& stream, ScanParameter& para)
	{
		uint32_t unknown1[3];
		readFStream(stream, unknown1, sizeof(unknown1)/sizeof(unknown1[0]));
		para.scanSizeXmm = readFStream<double>(stream);
		para.scanSizeYmm = readFStream<double>(stream);
		para.resZum      = readFStream<double>(stream);
	}

	void applyParamScan(const ScanParameter& para, BScanList& list)
	{
		if(list.size() == 0)
			return;

		const double resYmm = para.scanSizeYmm/static_cast<double>(list.size());
		const double resZmm = para.resZum/1000;
		for(BScanPair& bscan : list)
		{
			double resXmm = para.scanSizeXmm/static_cast<double>(bscan.image.cols);
			switch(bscan.data.bscanType)
			{
				case OctData::BScan::BScanType::Circle: bscan.data.scaleFactor = OctData::ScaleFactor(resXmm*M_PI, resYmm, resZmm); break;
				case OctData::BScan::BScanType::Line  : bscan.data.scaleFactor = OctData::ScaleFactor(resXmm     , resYmm, resZmm); break;
				case OctData::BScan::BScanType::Unknown: break;
			}

		}
	}

	void readConturInfo(std::istream& stream, BScanList& list, const OctData::FileReadOptions& op)
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

				emplace("RETINA_1", OctData::Segmentationlines::SegmentlineType::ILM);
				emplace("RETINA_2", OctData::Segmentationlines::SegmentlineType::PR1);
				emplace("RETINA_3", OctData::Segmentationlines::SegmentlineType::PR2);
				emplace("RETINA_4", OctData::Segmentationlines::SegmentlineType::BM );

				emplace("NFL_1", OctData::Segmentationlines::SegmentlineType::RNFL);
				emplace("NFL_2", OctData::Segmentationlines::SegmentlineType::IPL );
				emplace("NFL_3", OctData::Segmentationlines::SegmentlineType::OPL );
			}
		};

		static ConturInfo conturInfo;

		if(!op.readBScans)
			return;

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


		BOOST_LOG_TRIVIAL(debug) <<   "id     : " << id
		                         << "\ttype   : " << type
		                         << "\twidth  : " << width
		                         << "\tframes : " << frames
		                         << "\tsize   : " << size  ;
		if(type == 0)
		{
			std::unique_ptr<uint16_t[]> tempVector(new uint16_t[width]);
			uint16_t* tmpVec = tempVector.get();
			for(uint32_t frame = 0; frame < frames; ++frame)
			{
				uint32_t actFrame = frames-frame-1;
				if(actFrame < list.size())
				{
					BScanPair& bscanPair = list.at(actFrame);
					int imgHeight = bscanPair.image.rows;

					OctData::Segmentationlines::Segmentline line(width);
					readFStream(stream, tmpVec, width);
					std::transform(tmpVec, tmpVec + width, line.begin(), [imgHeight](uint16_t val){ return imgHeight - static_cast<OctData::Segmentationlines::SegmentlineDataType>(val); });
					bscanPair.data.getSegmentLine(lineType) = std::move(line);
				}
			}
		}
		else
			BOOST_LOG_TRIVIAL(error) << "Contuer info: unhandled type: " << type;
	}

	void applyRegistInfoVol(const uint32_t bound[4], BScanPair& bscan, double pos)
	{
		bscan.data.start = OctData::CoordSLOmm(bound[0]*pos + bound[2]*(1.-pos), bound[1]);
		bscan.data.end   = OctData::CoordSLOmm(bound[0]*pos + bound[2]*(1.-pos), bound[3]);
	}
	void applyRegistInfoCircle(const uint32_t bound[4], BScanPair& bscan)
	{
		bscan.data.start  = OctData::CoordSLOmm(bound[0] + bound[2], bound[1]);
		bscan.data.center = OctData::CoordSLOmm(bound[0]           , bound[1]);
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
			switch(bscan.data.bscanType)
			{
				case OctData::BScan::BScanType::Circle:
					switch(property.slotype)
					{
						case ReadProperty::SLOImageType::Unknown: break;
						case ReadProperty::SLOImageType::Fundus: applyRegistInfoCircle(boundFundus, bscan); break;
						case ReadProperty::SLOImageType::TRC:    applyRegistInfoCircle(boundTrc   , bscan); break;
					}
					break;
				case OctData::BScan::BScanType::Line:
					switch(property.slotype)
					{
						case ReadProperty::SLOImageType::Unknown: break;
						case ReadProperty::SLOImageType::Fundus: applyRegistInfoVol(boundFundus, bscan, pos); break;
						case ReadProperty::SLOImageType::TRC:    applyRegistInfoVol(boundTrc   , bscan, pos); break;
					}
					break;
				case OctData::BScan::BScanType::Unknown:
					break;
			}

			++bscanNum;
		}
	}


	void dumpChunk(std::istream& stream, const uint32_t chunkSize, const std::string& chunkName)
	{
		const std::streamoff chunkBegin  = stream.tellg();
		std::ofstream outStream(chunkName.substr(1), std::ios::binary);

		constexpr static const std::size_t buffSize = 2048;
		std::unique_ptr<char> buffer(new char[buffSize]);

		std::size_t bytesForCopy = chunkSize;

		while(bytesForCopy > 0)
		{
			std::size_t readBytes = std::min(bytesForCopy, buffSize);
			stream.read(buffer.get(), readBytes);
			outStream.write(buffer.get(), readBytes);
			bytesForCopy -= readBytes;
		}

		stream.seekg(chunkBegin);
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
		ScanParameter scanParameter;

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

			if(op.dumpFileParts)
				dumpChunk(stream, chunkSize, chunkName);

			const std::streamoff chunkBegin  = stream.tellg();
			BOOST_LOG_TRIVIAL(debug) << "chunkName "<< " (@" << chunkBegin << " -> " << static_cast<int>(chunkSize) << ") : " << chunkName ;

			if(chunkName == "@IMG_TRC_02")
				readImgTrc(stream, series, property);
			else if(chunkName == "@IMG_JPEG")
				readImgJpeg(stream, series, bscanList, callback, op);
			else if(chunkName == "@PATIENT_INFO_02")
				readPatientInfo02(stream, pat);
			else if(chunkName == "@PATIENT_INFO_03")
				readPatientInfo03(stream, pat, op);
			else if(chunkName == "@CONTOUR_INFO")
				readConturInfo(stream, bscanList, op);
			else if(chunkName == "@CAPTURE_INFO_02")
				readCaptureInfo02(stream, series);
			else if(chunkName == "@IMG_FUNDUS")
				readImgFundus(stream, series, property);
			else if(chunkName == "@REGIST_INFO")
				readRegistInfo(stream, bscanList, property);
			else if(chunkName == "@PARAM_SCAN_04")
				readParamScan04(stream, scanParameter);

			stream.seekg(chunkBegin + chunkSize);
		}

		applyParamScan(scanParameter, bscanList);

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
