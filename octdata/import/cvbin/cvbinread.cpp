#include "cvbinread.h"
#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include <filereadoptions.h>


#include <boost/log/trivial.hpp>

#include <cpp_framework/cvmat/cvmattreestruct.h>
#include <cpp_framework/cvmat/cvmattreestructextra.h>
#include <cpp_framework/cvmat/treestructbin.h>
#include <cpp_framework/callback.h>
#include <octfileread.h>


namespace bfs = boost::filesystem;


namespace OctData
{

	namespace
	{
		class TreeImgGetter
		{
			cv::Mat createdImage;
			bool extractRLE(const CppFW::CVMatTree* imgCompressNode)
			{
				int height = createdImage.rows;
				int width  = createdImage.cols;
				const std::size_t imageSize = static_cast<std::size_t>(height * width);


				const cv::Mat& compressSymbols   = imgCompressNode->getDirNode("compressSymbols")  .getMat();
				const cv::Mat& compressRunLength = imgCompressNode->getDirNode("compressRunLength").getMat();


				if(cv::DataType<uint8_t> ::type != compressSymbols  .type()
				|| cv::DataType<uint32_t>::type != compressRunLength.type())
					return false;

				if(compressSymbols.rows*compressSymbols.cols != compressRunLength.rows*compressRunLength.cols)
					return false;

				const int compDataLength = compressSymbols.rows*compressSymbols.cols;

				      uint8_t * imgPtr        = createdImage     .ptr<uint8_t >();
				const uint8_t * compSymbolPtr = compressSymbols  .ptr<uint8_t >();
				const uint32_t* compRunLenPtr = compressRunLength.ptr<uint32_t>();

				std::size_t imagePos = 0;
				for(int i = 0; i < compDataLength; ++i)
				{
					uint8_t  symbol = *compSymbolPtr;
					uint32_t number = *compRunLenPtr;

					if(imagePos >= imageSize - number)
						number = static_cast<uint32_t>(imageSize - imagePos - 1);

					std::memset(imgPtr, symbol, number);

					imagePos    += number;
					imgPtr      += number;
					++compSymbolPtr;
					++compRunLenPtr;
				}
				return true;
			}

			bool extractRLEErrorHandling(const CppFW::CVMatTree* imgCompressNode)
			{
				try
				{
					return extractRLE(imgCompressNode);
				}
				catch(CppFW::CVMatTree::WrongType& e)
				{
					BOOST_LOG_TRIVIAL(error) << "extractRLE: Wrong type : " << e.what();
				}
				catch(std::out_of_range& e)
				{
					BOOST_LOG_TRIVIAL(error) << "extractRLE: 0ut of range : " << e.what();
				}
				return false;
			}

		public:

			const cv::Mat* getCompreessMat(const CppFW::CVMatTree* bscanNode)
			{
				const CppFW::CVMatTree* imgCompressNode = bscanNode->getDirNodeOpt("imgCompress");

				if(!imgCompressNode || imgCompressNode->type() != CppFW::CVMatTree::Type::Dir)
					return nullptr;

				const CppFW::CVMatTree* compressMethodNode = imgCompressNode->getDirNodeOpt("compressMethod");

				if(!compressMethodNode || compressMethodNode->type() != CppFW::CVMatTree::Type::String)
					return nullptr;


				if(compressMethodNode->getString() != "RLE") // Lauflängenkodierung / run-length encoding
				{
					BOOST_LOG_TRIVIAL(error) << "Unknown compress method " << compressMethodNode->getString();
					return nullptr;
				}

				int imageHeight = CppFW::CVMatTreeExtra::getCvScalar(imgCompressNode, "height", uint32_t());
				int imageWidth  = CppFW::CVMatTreeExtra::getCvScalar(imgCompressNode, "width" , uint32_t());
				if(imageHeight == 0 || imageWidth == 0)
					return nullptr;

				createdImage = cv::Mat(imageHeight, imageWidth, cv::DataType<uint8_t>::type);

				if(!extractRLEErrorHandling(imgCompressNode))
					return nullptr;

				return &createdImage;
			}

			const cv::Mat* get(const CppFW::CVMatTree* bscanNode)
			{
				const CppFW::CVMatTree* seriesImgNode = bscanNode->getDirNodeOpt("img");

				if(!seriesImgNode || seriesImgNode->type() != CppFW::CVMatTree::Type::Mat)
					return getCompreessMat(bscanNode);

				return &(seriesImgNode->getMat());
			}

			void setBScanCoords(BScan::Data& bscanData, const CppFW::CVMatTree* bscanNode)
			{
				const CppFW::CVMatTree* bscanCoordNode = bscanNode->getDirNodeOpt("Coords");
				if(bscanCoordNode)
				{
					const CppFW::CVMatTree* bscanCoordStartNode = bscanCoordNode->getDirNodeOpt("Start");
					const CppFW::CVMatTree* bscanCoordEndNode   = bscanCoordNode->getDirNodeOpt("End");
					if(bscanCoordStartNode)
					{
						bscanData.start = CoordSLOmm(CppFW::CVMatTreeExtra::getCvScalar(bscanCoordStartNode, double(), 0)
						                           , CppFW::CVMatTreeExtra::getCvScalar(bscanCoordStartNode, double(), 1));
					}
					if(bscanCoordEndNode)
					{
						bscanData.end = CoordSLOmm(CppFW::CVMatTreeExtra::getCvScalar(bscanCoordEndNode, double(), 0)
						                         , CppFW::CVMatTreeExtra::getCvScalar(bscanCoordEndNode, double(), 1));
					}
				}
			}
		};

		class OctDataImgGetter
		{
			const Series& refSeries;
			std::size_t pos = 0;
			const BScan* actBscan;

		public:
			OctDataImgGetter(const Series& refSeries) : refSeries(refSeries) {}

			const cv::Mat* get(const CppFW::CVMatTree* /*bscanNode*/)
			{
				if(refSeries.bscanCount() <= pos)
				{
					BOOST_LOG_TRIVIAL(warning) << "to less BScans in refFile: " << pos << " / " << refSeries.bscanCount();
					return nullptr;
				}
				actBscan = refSeries.getBScan(pos);
				++pos;
				if(!actBscan)
					return nullptr;
				return &(actBscan->getImage());
			}

			void setBScanCoords(BScan::Data& bscanData, const CppFW::CVMatTree* /*bscanNode*/)
			{
				if(actBscan)
				{
					bscanData.start  = actBscan->getStart ();
					bscanData.end    = actBscan->getEnd   ();
					bscanData.center = actBscan->getCenter();
				}
			}

		};

		template<typename ImgGetter>
		bool fillSeries(const CppFW::CVMatTree::NodeList& seriesList, Series& series, ImgGetter imgGetter, CppFW::Callback* callback)
		{
			CppFW::CallbackStepper bscanCallbackStepper(callback, seriesList.size());
			for(const CppFW::CVMatTree* bscanNode : seriesList)
			{
				if(++bscanCallbackStepper == false)
					return false;

				if(!bscanNode)
					continue;

				const CppFW::CVMatTree* seriesSegNode = bscanNode->getDirNodeOpt("Segmentations");

				const cv::Mat* image = imgGetter.get(bscanNode);

				BScan::Data bscanData;
				if(seriesSegNode)
				{
					for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
					{
						const char* segLineName = Segmentationlines::getSegmentlineName(type);

						const CppFW::CVMatTree* seriesSegILMNode = seriesSegNode->getDirNodeOpt(segLineName);
						if(seriesSegILMNode && seriesSegILMNode->type() == CppFW::CVMatTree::Type::Mat)
						{
							const cv::Mat& segMat = seriesSegILMNode->getMat();
							cv::Mat convertedSegMat;
							segMat.convertTo(convertedSegMat, cv::DataType<double>::type);

							const double* p = convertedSegMat.ptr<double>(0);
							std::vector<double> segVec(p, p + convertedSegMat.rows*convertedSegMat.cols);

							bscanData.getSegmentLine(type) = std::move(segVec);
						}
					}
				}

				imgGetter.setBScanCoords(bscanData, bscanNode);

				BScan* bscan = new BScan(*image, bscanData);
				series.takeBScan(bscan);
			}
			return true;
		}

		OCT openIfExists(const bfs::path& file, const FileReadOptions& op, CppFW::Callback& callback)
		{
			if(bfs::exists(file))
				return OctFileRead::openFile(file.generic_string(), op, &callback);
			return OCT();
		}

		OCT tryOpenFile(const bfs::path& baseFile, const std::string& refFilename, const FileReadOptions& op, CppFW::Callback& callback)
		{
			bfs::path refFile(refFilename);
			bfs::path baseFilePath(baseFile.branch_path());

			// direkt path
			OCT octData = openIfExists(refFilename, op, callback);
			if(octData.size() > 0)
				return octData;

			// as rel path from base file
			bfs::path relPath = baseFilePath / refFile;
			octData = openIfExists(relPath, op, callback);
			if(octData.size() > 0)
				return octData;

			// only filename in base file path
			bfs::path filenamePath = baseFilePath / refFile.filename();
			octData = openIfExists(filenamePath, op, callback);

			return octData;
		}

	}

	CvBinRead::CvBinRead()
	: OctFileReader(OctExtension(".bin", "CvBin format"))
	{
	}

	bool CvBinRead::readFile(const bfs::path& file, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback)
	{
//
//     BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
//     BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
//     BOOST_LOG_TRIVIAL(info) << "An informational severity message";
//     BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
//     BOOST_LOG_TRIVIAL(error) << "An error severity message";
//     BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

		if(file.extension() != ".bin")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as bins";

		CppFW::CVMatTree octtree = CppFW::CVMatTreeStructBin::readBin(file.generic_string());

		if(octtree.type() != CppFW::CVMatTree::Type::Dir)
		{
			BOOST_LOG_TRIVIAL(trace) << "false internal structure of bin";
			return false;
		}


		const CppFW::CVMatTree* seriesNode = octtree.getDirNodeOpt("Serie");
		if(!seriesNode || seriesNode->type() != CppFW::CVMatTree::Type::List)
		{
			BOOST_LOG_TRIVIAL(trace) << "Serie node not found or false datatype";
			return false;
		}

		const CppFW::CVMatTree* patDataNode    = octtree.getDirNodeOpt("PatientData");
		const CppFW::CVMatTree* studyDataNode  = octtree.getDirNodeOpt("StudyData"  );
		const CppFW::CVMatTree* seriesDataNode = octtree.getDirNodeOpt("SeriesData" );

		int patId    = CppFW::CVMatTreeExtra::getCvScalar(patDataNode   , "ID", 1);
		int studyId  = CppFW::CVMatTreeExtra::getCvScalar(studyDataNode , "ID", 1);
		int seriesId = CppFW::CVMatTreeExtra::getCvScalar(seriesDataNode, "ID", 1);

		Patient& pat    = oct  .getPatient(patId   );
		Study&   study  = pat  .getStudy  (studyId );
		Series&  series = study.getSeries (seriesId);

		series.setRefSeriesUID(CppFW::CVMatTreeExtra::getStringOrEmpty(seriesDataNode, "RefUID"));
		series.setSeriesUID   (CppFW::CVMatTreeExtra::getStringOrEmpty(seriesDataNode, "UID"   ));
		pat   .setId          (CppFW::CVMatTreeExtra::getStringOrEmpty(patDataNode   , "PatID" ));


		bool fillStatus = false;

		const CppFW::CVMatTree::NodeList& seriesList = seriesNode->getNodeList();

		bool internalImgData = true;
		const CppFW::CVMatTree* imgFileNameNode = octtree.getDirNodeOpt("imgFileName" );
		if(imgFileNameNode)
			internalImgData = (imgFileNameNode->type() != CppFW::CVMatTree::Type::String);

		bool sloIsSet = false;
		const CppFW::CVMatTree* sloNode = octtree.getDirNodeOpt("slo");
		if(sloNode && sloNode->type() == CppFW::CVMatTree::Type::Mat)
		{
			SloImage* sloImage = new SloImage();
			sloImage->setImage(sloNode->getMat());
			sloImage->setScaleFactor(ScaleFactor(CppFW::CVMatTreeExtra::getCvScalar<double>(seriesDataNode, "SloScale", 1.0, 0)
			                                   , CppFW::CVMatTreeExtra::getCvScalar<double>(seriesDataNode, "SloScale", 1.0, 1)));
			series.takeSloImage(sloImage);
			sloIsSet = true;
		}


		if(internalImgData || !op.loadRefFiles)
			fillStatus = fillSeries(seriesList, series, TreeImgGetter(), callback);
		else
		{
			// *************
			// read ref file
			// *************
			CppFW::Callback callbackFake;
			if(!callback)
				callback = &callbackFake; // ensure that callback is not nullptr

			CppFW::Callback callbackRefLoad    = callback->createSubTask(80,  0);
			CppFW::Callback callbackFillSeries = callback->createSubTask(20, 80);

			FileReadOptions refFileOptions = op;
			refFileOptions.loadRefFiles = false;
// 			OCT refOct = OctFileRead::openFile(imgFileNameNode->getString(), refFileOptions, &callbackRefLoad);
			OCT refOct = tryOpenFile(file, imgFileNameNode->getString(), refFileOptions, callbackRefLoad);

			// TODO: multible files
			if(refOct.size() > 0)
			{
				const Patient* refPat = refOct.begin()->second;
				if(refPat && refPat->size() > 0)
				{
					const Study* refStudy  = refPat->begin()->second;
					if(refStudy && refStudy->size() > 0)
					{
						const Series* refSeries = refStudy->begin()->second;
						if(refSeries)
						{
							OctDataImgGetter imgGetter(*refSeries);
							fillStatus = fillSeries(seriesList, series, imgGetter, &callbackFillSeries);

							if(!sloIsSet)
							{
								SloImage* sloImage = new SloImage();
								sloImage->setImage(refSeries->getSloImage().getImage());
								sloImage->setScaleFactor(refSeries->getSloImage().getScaleFactor());
								series.takeSloImage(sloImage);
							}
						}
					}
				}
			}
		}



		if(fillStatus)
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" finished";
		else
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" failed";

		return fillStatus;
	}

	CvBinRead* CvBinRead::getInstance()
	{
		static CvBinRead instance;
		return &instance;
	}


}
