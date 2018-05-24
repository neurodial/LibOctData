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

template<typename T> struct GIPLFilterType;
template<> struct GIPLFilterType<uint8_t >{ constexpr static const int typeId = 8 ; };
template<> struct GIPLFilterType<uint16_t>{ constexpr static const int typeId = 15; };


namespace OctData
{

	namespace
	{
		template<typename T>
		void readFStream(std::istream& stream, T* dest, std::size_t num = 1)
		{
			stream.read(reinterpret_cast<char*>(dest), sizeof(T)*num);
		}

		class PrintObj
		{
			std::ostream& stream;
		public:
			PrintObj(std::ostream& stream) : stream(stream) {}

			template<typename T, typename P>
			void op(const char* name, const P& value, std::size_t /*size*/)
			{
				stream << name << ": " << value << std::endl;
			}

			template<typename T, typename P>
			void op(const char* name, const P* value, std::size_t size)
			{
				stream << name << ": ";
				for(std::size_t i = 0; i < size; ++i)
					stream << value[i] << "; ";
				stream << std::endl;
			}
		};

		class ReadFromStream
		{
			FileReader& filereader;

			template<typename T>
			void readOb(T& v, std::size_t size)
			{
				filereader.readFStreamBig(&v, size);
			}

			template<typename T>
			void readOb(T* v, std::size_t size)
			{
				filereader.readFStreamBig(v, size);
			}

		public:
			ReadFromStream(FileReader& filereader) : filereader(filereader) {}

			template<typename T, typename P>
			void op(const char* /*name*/, P& value, std::size_t size)
			{
				readOb(value, size);
			}
		};
		template<> void ReadFromStream::readOb(std::string& v, std::size_t size) { filereader.readString(v, size); }


		class GiplHeader
		{
		public:
			struct RawData
			{
				uint16_t    sizes        [ 4];
				uint16_t    image_type       ;
				float       scales       [ 4];
				std::string patient          ;
				float       matrix       [20];
				uint8_t     orientation      ;
				uint8_t     par2             ;
				double      voxmin           ;
				double      voxmax           ;
				double      origin       [ 4];
				float       pixval_offset    ;
				float       pixval_cal       ;
				float       interslicegap    ;
				float       user_def2        ;
				uint32_t    magic_number     ;
			};


			uint16_t getSizeX () const                                     { return header.sizes[0]  ; }
			uint16_t getSizeY () const                                     { return header.sizes[1]  ; }
			uint16_t getSizeZ () const                                     { return header.sizes[2]  ; }
			uint16_t getType  () const                                     { return header.image_type; }
			void print(std::ostream& stream) const
			{
				PrintObj p(stream);
				getSetParameter(p, *this);
			}

			void readInfo(FileReader& filereader)
			{
				ReadFromStream r(filereader);
				getSetParameter(r, *this);
			}

			bool numberCheck() const
			{
				return header.magic_number == 4026526128;
			}

		private:
			RawData header;

			template<typename T, typename VI>
			static void getSetParameter(T& getset, VI& data)
			{
				getset.template op<uint16_t>("sizes        ", data.header.sizes        , 4);
				getset.template op<uint16_t>("image_type   ", data.header.image_type   , 1);
				getset.template op<float   >("scales       ", data.header.scales       , 4);
				getset.template op<char    >("patient      ", data.header.patient      ,80);
				getset.template op<float   >("matrix       ", data.header.matrix       ,20);
				getset.template op<uint8_t >("orientation  ", data.header.orientation  , 1);
				getset.template op<uint8_t >("par2         ", data.header.par2         , 1);
				getset.template op<double  >("voxmin       ", data.header.voxmin       , 1);
				getset.template op<double  >("voxmax       ", data.header.voxmax       , 1);
				getset.template op<double  >("origin       ", data.header.origin       , 4);
				getset.template op<float   >("pixval_offset", data.header.pixval_offset, 1);
				getset.template op<float   >("pixval_cal   ", data.header.pixval_cal   , 1);
				getset.template op<float   >("interslicegap", data.header.interslicegap, 1);
				getset.template op<float   >("user_def2    ", data.header.user_def2    , 1);
				getset.template op<uint32_t>("magic_number ", data.header.magic_number , 1);
			}
		};
	}

	struct ReadUInt8
	{
		static void readImg(FileReader& filereader, cv::Mat& image, std::size_t sizeX, std::size_t sizeY) { filereader.readCVImage<uint8_t>(image, sizeY, sizeX); }
	};
	struct ReadUInt16
	{
		static void readImg(FileReader& filereader, cv::Mat& image, std::size_t sizeX, std::size_t sizeY)
		{
			cv::Mat tmpImg;
			filereader.readCVImage<uint16_t>(tmpImg, sizeY, sizeX);
			auto imgIt    = tmpImg.begin<uint16_t>();
			auto imgItEnd = tmpImg.end  <uint16_t>();

			for(;imgIt != imgItEnd; ++imgIt)
				boost::endian::big_to_native_inplace(*imgIt);

			tmpImg.convertTo(image, cv::DataType<uint8_t>::type, 1./4.);
		}
	};

	template<typename T>
	void readBScans(FileReader& filereader, Series& series, const OctData::FileReadOptions& op, const GiplHeader& giplHeader, CppFW::Callback* callback, T reader)
	{
		const std::size_t sizeX     = giplHeader.getSizeX();
		const std::size_t sizeY     = giplHeader.getSizeY();
		const std::size_t numBScans = giplHeader.getSizeZ();

		for(std::size_t numBscan = 0; numBscan<numBScans; ++numBscan)
		{
			if(callback)
				callback->callback(static_cast<double>(numBscan)/static_cast<double>(numBScans));

			cv::Mat bscanImage;
			reader.readImg(filereader, bscanImage, sizeX, sizeY);
// 			filereader.readCVImage<uint8_t>(bscanImage, sizeY, sizeX);


			BScan::Data bscanData;
			BScan* bscan = new BScan(bscanImage, bscanData);
			if(op.holdRawData)
				bscan->setRawImage(bscanImage);
			series.takeBScan(bscan);
		}
	}

	bool OctData::GIPLRead::readFile(FileReader& filereader, OctData::OCT& oct, const OctData::FileReadOptions& op, CppFW::Callback* callback)
	{
		if(filereader.getExtension() != ".gipl")
			return false;

		const std::string filename = filereader.getFilepath().generic_string();
		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as gpil";


		if(!filereader.openFile())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open vol file " << filename;
			return false;
		}

		BOOST_LOG_TRIVIAL(debug) << "open " << filename << " as gpil file";


		GiplHeader giplHeader;
		giplHeader.readInfo(filereader);
		giplHeader.print(std::cout);
		if(!giplHeader.numberCheck())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open vol file " << filename;
			return false;
		}


		if(giplHeader.getType() != GIPLFilterType<uint8_t >::typeId
		&& giplHeader.getType() != GIPLFilterType<uint16_t>::typeId)
		{
			BOOST_LOG_TRIVIAL(error) << "Non supported format (only uint8 and uint16 supported) " << giplHeader.getType();
			return false;
		}

		filereader.seekg(GIPL_HEADERSIZE);


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1); // TODO

		switch(giplHeader.getType())
		{
			case GIPLFilterType<uint8_t >::typeId: readBScans(filereader, series, op, giplHeader, callback, ReadUInt8 ()); break;
			case GIPLFilterType<uint16_t>::typeId: readBScans(filereader, series, op, giplHeader, callback, ReadUInt16()); break;
		}

		return true;
	}


	OctData::GIPLRead::GIPLRead()
	: OctFileReader(OctExtension{".gipl", ".gipl.gz", "Guys Image Processing Lab Format"})
	{
	}


}
