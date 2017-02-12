#include "tiffstackread.h"
#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include <filereadoptions.h>


#include <tiffio.h>
#include <tiffio.hxx>

namespace bfs = boost::filesystem;

namespace OctData
{

	namespace
	{
		template<typename T>
		void readTiffSeries(TIFF* tif, const FileReadOptions& op, uint32_t imageLength, uint32_t imageWidth, uint32_t sampleperpixel, Series& series, int& dircount, double alpha = 1, double beta = 0)
		{
			do {
				cv::Mat bscanImage(imageLength, imageWidth, CV_MAKETYPE(cv::DataType<T>::type, sampleperpixel));

				for(uint32 row = 0; row < imageLength; row++)
					TIFFReadScanline(tif, bscanImage.ptr<T>(row), row);

				if(sampleperpixel != 1)
					cv::cvtColor(bscanImage, bscanImage, CV_BGR2GRAY);

				cv::Mat bscanImageConvertedU8;
				bscanImage.convertTo(bscanImageConvertedU8, cv::DataType<uint8_t>::type, alpha, beta);
				// std::cout << "Tiff-Result: " << result << std::endl;
				BScan::Data bscanData;
				BScan* bscan = new BScan(bscanImageConvertedU8, bscanData);
				if(op.holdRawData)
					bscan->setRawImage(bscanImage);
				series.takeBScan(bscan);

				dircount++;
			} while(TIFFReadDirectory(tif));
		}
	}


	TiffStackRead::TiffStackRead()
	: OctFileReader(OctExtension{".tiff", ".tif", "Tiff stack"})
	{
	}

	bool TiffStackRead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* /*callback*/)
	{
		if(file.extension() != ".tiff" && file.extension() != ".tif")
			return false;
		
		if(!bfs::exists(file))
			return false;

		int dircount = 0;

		TIFF* tif = TIFFOpen(file.generic_string().c_str(), "r");
		if(tif)
		{
			Patient& pat    = oct  .getPatient(1);
			Study  & study  = pat  .getStudy(1);
			Series & series = study.getSeries(1);

			// http://www.libtiff.org/man/TIFFGetField.3t.html
			
			uint32 imageWidth, imageLength;
			uint32 tileWidth, tileLength;

			uint16 sampleFormat;
			uint16 sampleperpixel, bitspersample;

			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH , &imageWidth );
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imageLength);
			TIFFGetField(tif, TIFFTAG_TILEWIDTH  , &tileWidth  );
			TIFFGetField(tif, TIFFTAG_TILELENGTH , &tileLength );
			
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);   // get number of channels per pixel
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE  , &bitspersample );   // get the size of the channels

			if(TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT   , &sampleFormat  ) != 1)   // get data type
				sampleFormat = SAMPLEFORMAT_UINT;

			if(bitspersample == 8 && sampleFormat == SAMPLEFORMAT_UINT)
			{
				do {
					cv::Mat bscanImage(imageLength, imageWidth, CV_MAKETYPE(cv::DataType<unsigned char>::type, sampleperpixel));

					for(uint32 row = 0; row < imageLength; row++)
						TIFFReadScanline(tif, bscanImage.ptr<uint8_t>(row), row);

					if(sampleperpixel != 1)
						cv::cvtColor(bscanImage, bscanImage, CV_BGR2GRAY);

					// std::cout << "Tiff-Result: " << result << std::endl;
					BScan::Data bscanData;
					BScan* bscan = new BScan(bscanImage, bscanData);
					series.takeBScan(bscan);

					dircount++;
				} while (TIFFReadDirectory(tif));
			}
			else if(bitspersample == 16 && sampleFormat == SAMPLEFORMAT_UINT)
				readTiffSeries<uint16_t>(tif, op, imageLength, imageWidth, sampleperpixel, series, dircount, 1/255);
			else if(bitspersample == 32 && sampleFormat == SAMPLEFORMAT_IEEEFP)
				readTiffSeries<float>(tif, op, imageLength, imageWidth, sampleperpixel, series, dircount, 255);

			TIFFClose(tif);
		}

		return dircount>0;
	}

	TiffStackRead* TiffStackRead::getInstance()
	{
		static TiffStackRead instance; return &instance;
	}


}
