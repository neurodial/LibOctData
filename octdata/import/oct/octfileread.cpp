#include "octfileread.h"

#include <cpp_framework/callback.h>

#include <datastruct/oct.h>
#include <datastruct/bscan.h>
#include <datastruct/date.h>

#include <ostream>
#include <fstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include "../platform_helper.h"
#include <filereadoptions.h>


#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include<filereader/filereader.h>

namespace bfs = boost::filesystem;


namespace
{
	template<typename T>
	void readFStream(std::istream& stream, T* dest, std::size_t num = 1)
	{
		stream.read(reinterpret_cast<char*>(dest), sizeof(T)*num);
	}

	template<typename T>
	std::size_t readString(std::istream& stream, std::basic_string<T>& string, std::size_t maxChars)
	{
		T ch;
		string.reserve(maxChars);
		std::size_t charRead;
		for(charRead = 0; charRead < maxChars; ++charRead)
		{
			readFStream(stream, &ch);
			if(ch == 0)
				break;
			string.push_back(ch);
		}
		return charRead;
	}

	inline uint32_t readDatafieldLength(std::istream& stream)
	{
		uint32_t length;
		readFStream(stream, &length);
		return length;
	}


	template<typename T>
	std::size_t readCVImage(std::istream& stream, cv::Mat& image, std::size_t sizeX, std::size_t sizeY)
	{
		std::size_t datalength = readDatafieldLength(stream);

		image = cv::Mat(static_cast<int>(sizeX), static_cast<int>(sizeY), cv::DataType<T>::type);

		std::size_t num = sizeX*sizeY;
		std::size_t readLength = std::min(num*sizeof(T), datalength);

		stream.read(reinterpret_cast<char*>(image.data), readLength);

		return readLength + 4;
	}


	std::string readHeaderString(std::istream& stream)
	{
		uint32_t length = readDatafieldLength(stream);
		std::string str;
		readString(stream, str, length);
		return str;
	}

	uint32_t readRaw(std::istream& stream)
	{
		uint32_t length = readDatafieldLength(stream);
		stream.seekg(length, std::ios_base::cur);

		return length + 4;
	}

	std::size_t readRaw(std::istream& stream, void* dest, std::size_t maxLength, std::size_t& readedBytes)
	{
		std::size_t length = readDatafieldLength(stream);
		std::size_t readLength = std::min(length, maxLength);

		if(length != maxLength)
		{
			std::size_t absPos = stream.tellg();
			std::size_t relPos = readedBytes;
			BOOST_LOG_TRIVIAL(warning) << "wrong raw read size (" << length << " != " << maxLength << " bytes) AbsPos: " << absPos << " relPos: " << relPos;
		}

		stream.read(reinterpret_cast<char*>(dest), readLength);

		if(length > readLength)
			stream.seekg(length-readLength, std::ios_base::cur);

		readedBytes += length + 4;
		return length;
	}


	template<typename T>
	void fillValue(std::istream& stream, T& obj, std::size_t& readedBytes, const char* name)
	{
		uint32_t length = readDatafieldLength(stream);
		if(length != sizeof(T))
			BOOST_LOG_TRIVIAL(error) << "Reading wrong filetype by readValue: " << name;

		stream.read(reinterpret_cast<char*>(&obj), length);
		readedBytes += length + 4;
	}

	template<>
	void fillValue(std::istream& stream, std::string& obj, std::size_t& readedBytes, const char* /*name*/)
	{
// 		std::string str = readHeaderString(stream);
// 		std::cout << str << std::endl;
// 		std::cout << &obj << '\t' << obj << std::endl;
		obj = readHeaderString(stream);
		readedBytes += obj.length() + 4;
	}

	template<typename T>
	T readValue(std::istream& stream, std::size_t& readedBytes)
	{
		T obj;
		fillValue(stream, obj, readedBytes);

		return obj;
	}




	template<typename DictReader>
	std::size_t readDict(std::istream& stream, DictReader& reader, const std::size_t dictLength)
	{
		std::size_t bytesRead = 0;

		while(bytesRead < dictLength)
		{
			std::string keyStr = readHeaderString(stream);
			if(keyStr.empty())
				break;
			reader.handelDictEntry(stream, keyStr, bytesRead);
 			bytesRead += keyStr.length() + 4;
		}

		return bytesRead;
	}



	class DictFrameHeader
	{
		uint32_t    framecount    ;
		uint32_t    linecount     ;
		uint32_t    linelength    ;
		uint32_t    sampleformat  ;
		std::string description   ;
		double      xmin          ;
		double      xmax          ;
		std::string xcaption      ;
		double      ymin          ;
		double      ymax          ;
		std::string ycaption      ;
		uint32_t    scantype      ;
		double      scandepth     ;
		double      scanlength    ;
		double      azscanlength  ;
		double      elscanlength  ;
		double      objectdistance;
		double      scanangle     ;
		uint32_t    scans         ;
		uint32_t    frames        ;
		uint32_t    dopplerflag   ;


	public:
		void handelDictEntry(std::istream& stream, const std::string& name, std::size_t& readedBytes)
		{
			     if(name == "FRAMECOUNT"    ) fillValue(stream, framecount    , readedBytes, "FRAMECOUNT"    );
			else if(name == "LINECOUNT"     ) fillValue(stream, linecount     , readedBytes, "LINECOUNT"     );
			else if(name == "LINELENGTH"    ) fillValue(stream, linelength    , readedBytes, "LINELENGTH"    );
			else if(name == "SAMPLEFORMAT"  ) fillValue(stream, sampleformat  , readedBytes, "SAMPLEFORMAT"  );
			else if(name == "DESCRIPTION"   ) fillValue(stream, description   , readedBytes, "DESCRIPTION"   );
			else if(name == "XMIN"          ) fillValue(stream, xmin          , readedBytes, "XMIN"          );
			else if(name == "XMAX"          ) fillValue(stream, xmax          , readedBytes, "XMAX"          );
			else if(name == "XCAPTION"      ) fillValue(stream, xcaption      , readedBytes, "XCAPTION"      );
			else if(name == "YMIN"          ) fillValue(stream, ymin          , readedBytes, "YMIN"          );
			else if(name == "YMAX"          ) fillValue(stream, ymax          , readedBytes, "YMAX"          );
			else if(name == "YCAPTION"      ) fillValue(stream, ycaption      , readedBytes, "YCAPTION"      );
			else if(name == "SCANTYPE"      ) fillValue(stream, scantype      , readedBytes, "SCANTYPE"      );
			else if(name == "SCANDEPTH"     ) fillValue(stream, scandepth     , readedBytes, "SCANDEPTH"     );
			else if(name == "SCANLENGTH"    ) fillValue(stream, scanlength    , readedBytes, "SCANLENGTH"    );
			else if(name == "AZSCANLENGTH"  ) fillValue(stream, azscanlength  , readedBytes, "AZSCANLENGTH"  );
			else if(name == "ELSCANLENGTH"  ) fillValue(stream, elscanlength  , readedBytes, "ELSCANLENGTH"  );
			else if(name == "OBJECTDISTANCE") fillValue(stream, objectdistance, readedBytes, "OBJECTDISTANCE");
			else if(name == "SCANANGLE"     ) fillValue(stream, scanangle     , readedBytes, "SCANANGLE"     );
			else if(name == "SCANS"         ) fillValue(stream, scans         , readedBytes, "SCANS"         );
			else if(name == "FRAMES"        ) fillValue(stream, frames        , readedBytes, "FRAMES"        );
			else if(name == "DOPPLERFLAG"   ) fillValue(stream, dopplerflag   , readedBytes, "DOPPLERFLAG"   );
			else
			{
				std::size_t absPos = stream.tellg();
				std::size_t relPos = readedBytes;
				std::size_t rawLength = readRaw(stream);
				readedBytes += rawLength;
				BOOST_LOG_TRIVIAL(warning) << name << "\tuntreated (" << rawLength << " bytes) AbsPos: " << absPos << " relPos: " << relPos;
			}
		}

		uint32_t    getFramecount    () const                          { return framecount    ; }
		uint32_t    getLinecount     () const                          { return linecount     ; }
		uint32_t    getLinelength    () const                          { return linelength    ; }
		uint32_t    getSampleformat  () const                          { return sampleformat  ; }
		std::string getDescription   () const                          { return description   ; }
		double      getXmin          () const                          { return xmin          ; }
		double      getXmax          () const                          { return xmax          ; }
		std::string getXcaption      () const                          { return xcaption      ; }
		double      getYmin          () const                          { return ymin          ; }
		double      getYmax          () const                          { return ymax          ; }
		std::string getYcaption      () const                          { return ycaption      ; }
		uint32_t    getScantype      () const                          { return scantype      ; }
		double      getScandepth     () const                          { return scandepth     ; }
		double      getScanlength    () const                          { return scanlength    ; }
		double      getAzscanlength  () const                          { return azscanlength  ; }
		double      getElscanlength  () const                          { return elscanlength  ; }
		double      getObjectdistance() const                          { return objectdistance; }
		double      getScanangle     () const                          { return scanangle     ; }
		uint32_t    getScans         () const                          { return scans         ; }
		uint32_t    getFrames        () const                          { return frames        ; }
		uint32_t    getDopplerflag   () const                          { return dopplerflag   ; }

		void print(std::ostream& stream)
		{
			stream << "FRAMECOUNT    : " << framecount     << '\n';
			stream << "LINECOUNT     : " << linecount      << '\n';
			stream << "LINELENGTH    : " << linelength     << '\n';
			stream << "SAMPLEFORMAT  : " << sampleformat   << '\n';
			stream << "DESCRIPTION   : " << description    << '\n';
			stream << "XMIN          : " << xmin           << '\n';
			stream << "XMAX          : " << xmax           << '\n';
			stream << "XCAPTION      : " << xcaption       << '\n';
			stream << "YMIN          : " << ymin           << '\n';
			stream << "YMAX          : " << ymax           << '\n';
			stream << "YCAPTION      : " << ycaption       << '\n';
			stream << "SCANTYPE      : " << scantype       << '\n';
			stream << "SCANDEPTH     : " << scandepth      << '\n';
			stream << "SCANLENGTH    : " << scanlength     << '\n';
			stream << "AZSCANLENGTH  : " << azscanlength   << '\n';
			stream << "ELSCANLENGTH  : " << elscanlength   << '\n';
			stream << "OBJECTDISTANCE: " << objectdistance << '\n';
			stream << "SCANANGLE     : " << scanangle      << '\n';
			stream << "SCANS         : " << scans          << '\n';
			stream << "FRAMES        : " << frames         << '\n';
			stream << "DOPPLERFLAG   : " << dopplerflag    << '\n';
		}

		void copyData(OctData::Series& series)
		{
			series.setScanPattern(OctData::Series::ScanPattern::Text);
			series.setScanPatternText(std::string("Scantyp: ") + boost::lexical_cast<std::string>(scantype));

			series.setDescription(description);
		}
	};

	class DictFrameData
	{
		char     framedatetime[16];
		uint64_t frametimestamp;
		uint32_t framelines    ;

		OctData::Date date;
		OctData::BScan::Data bscanData;

		OctData::Series& series;
		const DictFrameHeader& dictFrameHeader;
		const OctData::FileReadOptions& op;
	public:
		DictFrameData(OctData::Series& series, const DictFrameHeader& dictFrameHeader, const OctData::FileReadOptions& op) : series(series), dictFrameHeader(dictFrameHeader), op(op) {}

		void handelDictEntry(std::istream& stream, const std::string& name, std::size_t& readedBytes)
		{
			     if(name == "FRAMELINES"    ) fillValue(stream, framelines    , readedBytes, "FRAMELINES"    );
			else if(name == "FRAMETIMESTAMP") fillValue(stream, frametimestamp, readedBytes, "FRAMETIMESTAMP");
			else if(name == "FRAMEDATETIME" )
			{
				const std::size_t dateFieldLength = sizeof(framedatetime)/sizeof(framedatetime[0]);
				std::size_t bytesRead = readRaw(stream, framedatetime, dateFieldLength, readedBytes);
				if(bytesRead == dateFieldLength)
				{
					date.setYear (*reinterpret_cast<uint16_t*>(framedatetime  ));
					date.setMonth(*reinterpret_cast<uint16_t*>(framedatetime+2));
					date.setDay  (*reinterpret_cast<uint16_t*>(framedatetime+4)); // TODO timeelement 6 ?
					date.setHour (*reinterpret_cast<uint16_t*>(framedatetime+8));
					date.setMin  (*reinterpret_cast<uint16_t*>(framedatetime+10));
					date.setSec  (*reinterpret_cast<uint16_t*>(framedatetime+12));
					date.setMs   (*reinterpret_cast<uint16_t*>(framedatetime+14));
					date.setDateAsValid();

					bscanData.acquisitionTime = date;
// 					std::cout << "date: " << date.timeDateStr() << std::endl;
				}
			}
 			else if(name == "FRAMESAMPLES"  )
			{
				cv::Mat rawImage, viewImage;

				switch(dictFrameHeader.getSampleformat())
				{
					case 1:
						readedBytes += readCVImage<uint8_t>(stream, viewImage, dictFrameHeader.getLinecount(), dictFrameHeader.getLinelength());
						break;
					case 2:
						readedBytes += readCVImage<uint16_t>(stream, rawImage, dictFrameHeader.getLinecount(), dictFrameHeader.getLinelength());
						rawImage.convertTo(viewImage, CV_8U, 1/255., 0);
						break;
					default:
						readRaw(stream);
						return;
				}

				OctData::BScan* bscan = new OctData::BScan(viewImage.t(), bscanData);
				if(op.holdRawData && !rawImage.empty())
					bscan->setRawImage(rawImage);
				series.takeBScan(bscan);
			}
			else
			{
				std::size_t absPos = stream.tellg();
				std::size_t relPos = readedBytes;
				std::size_t rawLength = readRaw(stream);
				readedBytes += rawLength;
				BOOST_LOG_TRIVIAL(warning) << name << "\tuntreated (" << rawLength << " bytes) AbsPos: " << absPos << " relPos: " << relPos;
			}
		}

		void print(std::ostream& stream)
		{
// 			stream << "FRAMEDATETIME  : " << framedatetime  << '\n';
			stream << "FRAMETIMESTAMP : " << frametimestamp << '\n';
			stream << "FRAMELINES     : " << framelines     << '\n';
		}
	};

	class MainDict
	{
		OctData::Series& series;
		const OctData::FileReadOptions& op;
		CppFW::CallbackStepper& callbackStepper;
		DictFrameHeader dictFrameHeader;
	public:
		MainDict(OctData::Series& series, const OctData::FileReadOptions& op, CppFW::CallbackStepper& callbackStepper) : series(series), op(op), callbackStepper(callbackStepper) {}

		void handelDictEntry(std::istream& stream, const std::string& name, std::size_t& readedBytes)
		{
			const uint32_t    dictLength = readDatafieldLength(stream);
			const std::size_t dictBegin  = stream.tellg();

			callbackStepper.setStep(dictBegin);

// 			std::cout << "Dict: \t" << name << std::endl;
			if(name == "FRAMEDATA")
			{
				DictFrameData dictFrameData(series, dictFrameHeader, op);
				readedBytes += readDict(stream, dictFrameData, dictLength);
			}
			else if(name == "FRAMEHEADER")
			{
				readedBytes += readDict(stream, dictFrameHeader, dictLength);
				dictFrameHeader.print(std::cout);
				dictFrameHeader.copyData(series);
			}
			else
			{
				BOOST_LOG_TRIVIAL(warning) << "Dict: " << name << " untreated (" << dictLength << " bytes)";
			}
			stream.seekg(dictBegin + dictLength);

		}
	};

}




namespace OctData
{

	OctFileFormatRead::OctFileFormatRead()
	: OctFileReader(OctExtension(".OCT", "Oct file"))
	{
	}

	bool OctFileFormatRead::readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();
//
//     BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
//     BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
//     BOOST_LOG_TRIVIAL(info) << "An informational severity message";
//     BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
//     BOOST_LOG_TRIVIAL(error) << "An error severity message";
//     BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

		if(file.extension() != ".OCT")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as oct file";

		boost::system::error_code ec;
		boost::uintmax_t filesize = file_size(file, ec);
		if(ec)
		{
			BOOST_LOG_TRIVIAL(error) << "An error severity message";
			filesize = 1;
		}
		CppFW::CallbackStepper callbackStepper(callback, filesize);


		std::fstream stream(filepathConv(file), std::ios::binary | std::ios::in);
		if(!stream.good())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open vol file " << filepathConv(file);
			return false;
		}

		BOOST_LOG_TRIVIAL(debug) << "open " << file.generic_string() << " as vol file";

		std::string dir      = file.branch_path().generic_string();
		std::string filename = file.filename   ().generic_string();

		const unsigned char magicHeader[6] = { 0xa5, 0xa7, 0x7d, 0x0c, 0x0a, 0x01 };


		const std::size_t formatstringlength = sizeof(magicHeader)/sizeof(magicHeader[0]);
		char fileformatstring[formatstringlength];
		readFStream(stream, fileformatstring, formatstringlength);
		if(memcmp(fileformatstring, magicHeader, formatstringlength) != 0) // 0 = strings are equal
		{
			BOOST_LOG_TRIVIAL(error) << file.generic_string() << " Wrong fileformat (wrong header)";
			return false;
		}


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1);



		MainDict mainDict(series, op, callbackStepper);

		stream.seekg(0, std::ios_base::end);
		std::size_t fielsize = stream.tellg();
		stream.seekg(6, std::ios_base::beg);

		readDict(stream, mainDict, fielsize);

		if(callback)
			callback->callback(1); // set to 100% ( = 1 frac)


		BOOST_LOG_TRIVIAL(debug) << "read oct file \"" << file.generic_string() << "\" finished";
		return true;
	}

}
