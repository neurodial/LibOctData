#include "cvbinread.h"

#include<locale>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include <opencv2/opencv.hpp>

#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <filereadoptions.h>


#include <cpp_framework/cvmat/cvmattreestruct.h>
#include <cpp_framework/cvmat/cvmattreestructextra.h>
#include <cpp_framework/cvmat/cvmattreegetset.h>
#include <cpp_framework/cvmat/treestructbin.h>
#include <cpp_framework/callback.h>
#include <octfileread.h>

#include<filereader/filereader.h>

namespace bfs = boost::filesystem;


namespace OctData
{

	namespace
	{

		const CppFW::CVMatTree* getDirNodeOptCamelCase(const CppFW::CVMatTree& node, const char* name)
		{
			const CppFW::CVMatTree* result = node.getDirNodeOpt(name);
			if(!result)
			{
				std::string nameFirstUpper(name);
				nameFirstUpper[0] = static_cast<std::string::value_type>(std::toupper(nameFirstUpper[0], std::locale()));
				result = node.getDirNodeOpt(nameFirstUpper.c_str());
			}
			return result;
		}


// 		const CppFW::CVMatTree* getDirNodeOptCamelCase(const CppFW::CVMatTree* node, const char* name)
// 		{
// 			if(node) return getDirNodeOptCamelCase(*node, name);
// 			return nullptr;
// 		}

		// general export methods
		SloImage* readSlo(const CppFW::CVMatTree* sloNode)
		{
			if(!sloNode)
				return nullptr;

			const CppFW::CVMatTree* imgNode = sloNode->getDirNodeOpt("img");
			if(imgNode && imgNode->type() == CppFW::CVMatTree::Type::Mat)
			{
				SloImage* slo = new SloImage();
				CppFW::GetFromCVMatTree sloWriter(*sloNode);
				slo->getSetParameter(sloWriter);
				slo->setImage(imgNode->getMat());
				return slo;
			}
			return nullptr;
		}

		/*

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
*/

		void fillSegmentationsLines(const CppFW::CVMatTree* segNode, BScan::Data& bscanData)
		{
			if(segNode)
			{
				for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
				{
					const char* segLineName = Segmentationlines::getSegmentlineName(type);

					const CppFW::CVMatTree* seriesSegILMNode = segNode->getDirNodeOpt(segLineName);
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
		}

		BScan* readBScan(const CppFW::CVMatTree* bscanNode)
		{
			if(!bscanNode)
				return nullptr;

			BScan* bscan = nullptr;
			const CppFW::CVMatTree* imgNode = bscanNode->getDirNodeOpt("img");
			if(imgNode && imgNode->type() == CppFW::CVMatTree::Type::Mat)
			{
				const cv::Mat& img = imgNode->getMat();
				BScan::Data bscanData;

				const CppFW::CVMatTree* seriesSegNode = getDirNodeOptCamelCase(*bscanNode, "segmentations");
				fillSegmentationsLines(seriesSegNode, bscanData);

				bscan = new BScan(img, bscanData);

				CppFW::GetFromCVMatTree bscanReader(bscanNode->getDirNodeOpt("data"));
				bscan->getSetParameter(bscanReader);


				const CppFW::CVMatTree* angioNode = bscanNode->getDirNodeOpt("angioImg");
				if(angioNode && angioNode->type() == CppFW::CVMatTree::Type::Mat)
					bscan->setAngioImage(angioNode->getMat());
			}

			return bscan;
		}

		bool readBScanList(const CppFW::CVMatTree::NodeList& seriesList, Series& series, CppFW::Callback* callback)
		{
			CppFW::CallbackStepper bscanCallbackStepper(callback, seriesList.size());
			for(const CppFW::CVMatTree* bscanNode : seriesList)
			{
				if(++bscanCallbackStepper == false)
					return false;

				BScan* bscan = readBScan(bscanNode);
				if(bscan)
					series.takeBScan(bscan);
			}
			return true;
		}




		// deep file format (support many scans per file, tree structure)
		template<typename S>
		bool readStructure(const CppFW::CVMatTree& tree, S& structure, CppFW::Callback* callback)
		{
			bool result = true;
			const CppFW::CVMatTree* dataNode = tree.getDirNodeOpt("data");
			if(dataNode)
			{
				CppFW::GetFromCVMatTree structureReader(dataNode);
				structure.getSetParameter(structureReader);
			}


			for(const CppFW::CVMatTree::NodePair& subNodePair : tree.getNodeDir())
			{
				if(!subNodePair.second)
					continue;

				const std::string& nodeName = subNodePair.first;
				if(nodeName.substr(0,3) == "id_")
				{
					const std::string& nodeIdStr = nodeName.substr(3, 50);

					try
					{
						int id = boost::lexical_cast<int>(nodeIdStr);
						readStructure(*(subNodePair.second), structure.getInsertId(id), callback);
					}
					catch(const boost::bad_lexical_cast&)
					{
					}
				}
			}
			return result;
		}


		template<>
		bool readStructure<Series>(const CppFW::CVMatTree& tree, Series& series, CppFW::Callback* callback)
		{
			const CppFW::CVMatTree* dataNode = tree.getDirNodeOpt("data");
			if(dataNode)
			{
				CppFW::GetFromCVMatTree seriesReader(dataNode);
				series.getSetParameter(seriesReader);
			}


			const CppFW::CVMatTree* sloNode = tree.getDirNodeOpt("slo");
			series.takeSloImage(readSlo(sloNode));

			const CppFW::CVMatTree* bscansNode = getDirNodeOptCamelCase(tree, "bscans");
			if(!bscansNode)
				return false;

			const CppFW::CVMatTree::NodeList& seriesList = bscansNode->getNodeList();
			return readBScanList(seriesList, series, callback);
		}

		bool readTreeData(OCT& oct, const CppFW::CVMatTree& octtree, CppFW::Callback* callback)
		{
			return readStructure(octtree, oct, callback);
		}




		bool readFlatData(OCT& oct, const CppFW::CVMatTree& octtree, const CppFW::CVMatTree* seriesNode, CppFW::Callback* callback)
		{
			if(seriesNode->type() != CppFW::CVMatTree::Type::List)
			{
				BOOST_LOG_TRIVIAL(trace) << "Serie node not found or false datatype";
				return false;
			}

			const CppFW::CVMatTree* patDataNode    = getDirNodeOptCamelCase(octtree, "patientData");
			const CppFW::CVMatTree* studyDataNode  = getDirNodeOptCamelCase(octtree, "studyData"  );
			const CppFW::CVMatTree* seriesDataNode = getDirNodeOptCamelCase(octtree, "seriesData" );

			int patId    = CppFW::CVMatTreeExtra::getCvScalar(patDataNode   , "ID", 1);
			int studyId  = CppFW::CVMatTreeExtra::getCvScalar(studyDataNode , "ID", 1);
			int seriesId = CppFW::CVMatTreeExtra::getCvScalar(seriesDataNode, "ID", 1);

			Patient& pat    = oct  .getPatient(patId   );
			Study&   study  = pat  .getStudy  (studyId );
			Series&  series = study.getSeries (seriesId);

			if(patDataNode)
			{
				CppFW::GetFromCVMatTree patientReader(*patDataNode);
				pat.getSetParameter(patientReader);
			}

			if(studyDataNode)
			{
				CppFW::GetFromCVMatTree studyReader(*studyDataNode);
				study.getSetParameter(studyReader);
			}

			if(seriesDataNode)
			{
				CppFW::GetFromCVMatTree seriesReader(*seriesDataNode);
				series.getSetParameter(seriesReader);
			}

			const CppFW::CVMatTree* sloNode = octtree.getDirNodeOpt("slo");
			series.takeSloImage(readSlo(sloNode));

			const CppFW::CVMatTree::NodeList& seriesList = seriesNode->getNodeList();
			return readBScanList(seriesList, series, callback);
		}

	}

	CvBinRead::CvBinRead()
	: OctFileReader(OctExtension(".octbin", "CvBin format"))
	{
	}

	bool CvBinRead::readFile(FileReader& filereader, OCT& oct, const FileReadOptions& /*op*/, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();
//
//     BOOST_LOG_TRIVIAL(trace)   << "A trace severity message";
//     BOOST_LOG_TRIVIAL(debug)   << "A debug severity message";
//     BOOST_LOG_TRIVIAL(info)    << "An informational severity message";
//     BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
//     BOOST_LOG_TRIVIAL(error)   << "An error severity message";
//     BOOST_LOG_TRIVIAL(fatal)   << "A fatal severity message";

		if(file.extension() != ".octbin" && file.extension() != ".bin")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as bins";

		CppFW::CVMatTree octtree = CppFW::CVMatTreeStructBin::readBin(file.generic_string());
		if(callback)
			callback->callback(0.5);

		if(octtree.type() != CppFW::CVMatTree::Type::Dir)
		{
			BOOST_LOG_TRIVIAL(trace) << "false internal structure of bin";
			return false;
		}

		bool fillStatus;
		const CppFW::CVMatTree* seriesNode = getDirNodeOptCamelCase(octtree, "serie");
		if(seriesNode)
			fillStatus = readFlatData(oct, octtree, seriesNode, callback);
		else
			fillStatus = readTreeData(oct, octtree, callback);



		if(fillStatus)
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" finished";
		else
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" failed";

		return fillStatus;

	}


}
