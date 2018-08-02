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
#include "topcondata.h"

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


	void readImgJpeg(std::istream& stream, TopconData& data, CppFW::Callback* callback, const OctData::FileReadOptions& op)
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
				data.series.setScanPattern(OctData::Series::ScanPattern::SingleLine);
				break;
			case 1:
				data.series.setScanPattern(OctData::Series::ScanPattern::Circular);
				bscanType = OctData::BScan::BScanType::Circle;
				break;
			case 2:
				data.series.setScanPattern(OctData::Series::ScanPattern::Volume);
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

			TopconData::BScanPair pair;
			pair.image = image;
			pair.data.bscanType = bscanType;
			data.bscanList.push_back(pair);

		}
	}

	enum class SLOType { Fundus, TRC };
	void readImgSlo(std::istream& stream, TopconData& data, SLOType sloType)
	{
		uint32_t u1;

		const uint32_t width  = readFStream<uint32_t>(stream);
		const uint32_t height = readFStream<uint32_t>(stream);
		const uint32_t bits   = readFStream<uint32_t>(stream);
		const uint32_t frames = readFStream<uint32_t>(stream);
		if(sloType == SLOType::TRC)
			u1 = readFStream<uint8_t >(stream);
		else
			u1 = readFStream<uint32_t>(stream);

		BOOST_LOG_TRIVIAL(debug) << "width  : " << width
		                 << '\t' << "height : " << height
		                 << '\t' << "bits   : " << bits
		                 << '\t' << "frames : " << frames
		                 << '\t' << "u1     : " << static_cast<int>(u1);

		// use only the first image
		const uint32_t size = readFStream<uint32_t>(stream);
		cv::Mat image = readAndEncodeJPEG2kData(stream, size);

		if(image.empty())
			return;

		switch(sloType)
		{
			case SLOType::Fundus:
				data.sloFundus.sloImage = new OctData::SloImage;
				data.sloFundus.sloImage->setImage(image);
				break;
			case SLOType::TRC:
				data.sloTRC.sloImage = new OctData::SloImage;
				data.sloTRC.sloImage->setImage(image);
				break;
		}
	}


	void decriptString(std::string& str, const std::array<unsigned char, 32>& key)
	{
		const std::size_t cryptLen = std::min(key.size(), str.size());
		for(std::size_t i = 0; i < cryptLen; ++i)
			str[i] = key[i]^static_cast<unsigned char>(str[i]);
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
			std::fstream stream(op.libPath + "/topcon_key.txt");
			keyLoaded = stream.good();
			if(keyLoaded)
				BOOST_LOG_TRIVIAL(info) << "read topcon keys from :" << op.libPath << "/topcon_key.txt";
			else
				BOOST_LOG_TRIVIAL(info) << "read topcon keys from :" << op.libPath << "/topcon_key.txt failed";

			readKey(keyID      , stream);
			readKey(keyForename, stream);
			readKey(keySurename, stream);
		}

		const std::array<unsigned char, 32>& getKeyID      ()          { return keyID      ; }
		const std::array<unsigned char, 32>& getKeyForename()          { return keyForename; }
		const std::array<unsigned char, 32>& getKeySurename()          { return keySurename; }

		bool isKeyLoaded()                                       const { return keyLoaded; }
	};

	void readPatientInfo0203(std::istream& stream, TopconData& data, const OctData::FileReadOptions& op, bool decrypt)
	{

		std::string patId;
		std::string patForeName;
		std::string patSureName;
		std::string zeros;
		readFStreamFull(stream, patId      , 32);
		readFStreamFull(stream, patForeName, 32);
		readFStreamFull(stream, patSureName, 32);
		readFStream(stream, zeros      , 8);

		readFStream<uint8_t>(stream);

		OctData::Date birthDate;

		if(decrypt)
		{
			EncryptKeyPatientInfo03 keys(op);

			if(!keys.isKeyLoaded())
				return;

			decriptString(patId      , keys.getKeyID      ());
			decriptString(patForeName, keys.getKeyForename());
			decriptString(patSureName, keys.getKeySurename());
		}
		else
		{
			birthDate.setYear (readFStream<uint16_t>(stream));
			birthDate.setMonth(readFStream<uint16_t>(stream));
			birthDate.setDay  (readFStream<uint16_t>(stream));
			birthDate.setDateAsValid();
		}


		stripZeroCaracters(patId      );
		stripZeroCaracters(patForeName);
		stripZeroCaracters(patSureName);

		data.pat.setId       (patId      );
		data.pat.setForename (patForeName);
		data.pat.setSurname  (patSureName);
		data.pat.setBirthdate(birthDate  );
	}

	void readCaptureInfo02(std::istream& stream, TopconData& data)
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

		data.series.setScanDate(scanDate);

		switch(lateralityByte)
		{
			case 0: data.series.setLaterality(OctData::Series::Laterality::OD); break;
			case 1: data.series.setLaterality(OctData::Series::Laterality::OS); break;
		}
	}

	void readParamScan04(std::istream& stream, TopconData& data)
	{
		uint32_t unknown1[3];
		readFStream(stream, unknown1, sizeof(unknown1)/sizeof(unknown1[0]));
		data.scanParameter.scanSizeXmm = readFStream<double>(stream);
		data.scanParameter.scanSizeYmm = readFStream<double>(stream);
		data.scanParameter.resZum      = readFStream<double>(stream);
	}




	void readConturInfo(std::istream& stream, TopconData& data, const OctData::FileReadOptions& op)
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

		TopconData::BScanList& list = data.bscanList;

		if(list.size() < frames)
			list.resize(frames);

		if(type == 0)
		{
			std::unique_ptr<uint16_t[]> tempVector(new uint16_t[width]);
			uint16_t* tmpVec = tempVector.get();
			for(uint32_t frame = 0; frame < frames; ++frame)
			{
				uint32_t actFrame = frames-frame-1;

				TopconData::BScanPair& bscanPair = list[actFrame];
				int imgHeight = bscanPair.image.rows;

				OctData::Segmentationlines::Segmentline line(width);
				readFStream(stream, tmpVec, width);
				std::transform(tmpVec, tmpVec + width, line.begin(), [imgHeight](uint16_t val){ return imgHeight - static_cast<OctData::Segmentationlines::SegmentlineDataType>(val); });
				bscanPair.data.getSegmentLine(lineType) = std::move(line);
			}
		}
		else
			BOOST_LOG_TRIVIAL(error) << "Contuer info: unhandled type: " << type;
	}


	void readRegistInfo(std::istream& stream, TopconData& data)
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

		data.sloTRC   .registData = TopconData::SloRegistData(boundTrc   );
		data.sloFundus.registData = TopconData::SloRegistData(boundFundus);

		/*
		double bound[4];
		switch(property.slotype)
		{
			case ReadProperty::SLOImageType::Unknown: return;
			case ReadProperty::SLOImageType::Fundus: std::copy(boundFundus, boundFundus+4, bound); break;
			case ReadProperty::SLOImageType::TRC:    std::copy(boundTrc   , boundTrc+4   , bound); break;
		}

		double scanSizeXpx = bound[2] - bound[0];
		double scanSizeYpx = bound[3] - bound[1];

		parameter.sloScaleX = parameter.scanSizeXmm/scanSizeXpx;
		parameter.sloScaleY = parameter.scanSizeYmm/scanSizeYpx;

		bound[0] *= parameter.sloScaleX;
		bound[1] *= parameter.sloScaleY;
		bound[2] *= parameter.sloScaleX;
		bound[3] *= parameter.sloScaleY;
*/

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

		TopconData data(oct);

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
				readImgSlo(stream, data, SLOType::TRC);
			else if(chunkName == "@IMG_JPEG")
				readImgJpeg(stream, data, callback, op);
			else if(chunkName == "@PATIENT_INFO_02")
				readPatientInfo0203(stream, data, op, false);
			else if(chunkName == "@PATIENT_INFO_03")
				readPatientInfo0203(stream, data, op, true);
			else if(chunkName == "@CONTOUR_INFO")
				readConturInfo(stream, data, op);
			else if(chunkName == "@CAPTURE_INFO_02")
				readCaptureInfo02(stream, data);
			else if(chunkName == "@IMG_FUNDUS")
				readImgSlo(stream, data, SLOType::Fundus);
			else if(chunkName == "@REGIST_INFO")
				readRegistInfo(stream, data);
			else if(chunkName == "@PARAM_SCAN_04")
				readParamScan04(stream, data);

			stream.seekg(chunkBegin + chunkSize);
		}

		BOOST_LOG_TRIVIAL(debug) << "read oct file \"" << file.generic_string() << "\" finished";
		return true;
	}

}
