#include "xoctread.h"

#include<locale>
#include<vector>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include<boost/property_tree/xml_parser.hpp>
#include<boost/interprocess/streams/bufferstream.hpp>

#include <opencv2/opencv.hpp>

#include<cpp_framework/callback.h>
#include<cpp_framework/zip/unzipcpp.h>


#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <filereadoptions.h>

#include <octfileread.h>
#include<filereader/filereader.h>

namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;
namespace bip = boost::interprocess;

namespace OctData
{
	namespace
	{

		class GetFromPTree
		{
			const bpt::ptree* tree;
		public:
			GetFromPTree(const bpt::ptree& tree) : tree(&tree) {}
			GetFromPTree(const bpt::ptree* tree) : tree(tree) {}

			template<typename T>
			void operator()(const std::string& name, T& value)
			{
				if(tree)
				{
					boost::optional<T> t = tree->get_optional<T>(name);
					if(t)
						value = std::move(*t);
				}
			}

			GetFromPTree subSet(const std::string& name)
			{
				if(tree)
				{
					boost::optional<const bpt::ptree&> subTree = tree->get_child_optional(name);
					if(subTree)
						return GetFromPTree(*subTree);
				}
				return GetFromPTree(nullptr);
			}
		};

// 		const CppFW::CVMatTree* getDirNodeOptCamelCase(const CppFW::CVMatTree& node, const char* name)
// 		{
// 			const CppFW::CVMatTree* result = node.getDirNodeOpt(name);
// 			if(!result)
// 			{
// 				std::string nameFirstUpper(name);
// 				nameFirstUpper[0] = static_cast<std::string::value_type>(std::toupper(nameFirstUpper[0], std::locale()));
// 				result = node.getDirNodeOpt(nameFirstUpper.c_str());
// 			}
// 			return result;
// 		}


// 		const CppFW::CVMatTree* getDirNodeOptCamelCase(const CppFW::CVMatTree* node, const char* name)
// 		{
// 			if(node) return getDirNodeOptCamelCase(*node, name);
// 			return nullptr;
// 		}

		template<typename S>
		void readDataNode(const bpt::ptree& tree, S& structure)
		{
			boost::optional<const bpt::ptree&> dataNode = tree.get_child_optional("data");
			if(dataNode)
			{
				GetFromPTree structureReader(*dataNode);
				structure.getSetParameter(structureReader);
			}
		}

		// general import methods
		SloImage* readSlo(const bpt::ptree& sloNode, CppFW::UnzipCpp& zipfile)
		{
			boost::optional<std::string> imagePath = sloNode.get_optional<std::string>("image");
			if(!imagePath)
				return nullptr;

			std::vector<char> sloContent = zipfile.readFile(*imagePath);
			if(sloContent.size() == 0)
				return nullptr;

			SloImage* slo = new SloImage();
			slo->setImage(cv::imdecode(sloContent, cv::IMREAD_UNCHANGED));
			readDataNode(sloNode, *slo);
			return slo;
		}
		/*

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
*/

		BScan* readBScan(const bpt::ptree& bscanNode, CppFW::UnzipCpp& zipfile)
		{
			boost::optional<std::string> imagePath = bscanNode.get_optional<std::string>("image");
			if(!imagePath)
				return nullptr;

			std::vector<char> bscanImageContent = zipfile.readFile(*imagePath);
			if(bscanImageContent.size() == 0)
				return nullptr;


// 			const CppFW::CVMatTree* angioNode = bscanNode->getDirNodeOpt("angioImg");
// 			if(angioNode && angioNode->type() == CppFW::CVMatTree::Type::Mat)
// 				bscan->setAngioImage(angioNode->getMat());


			cv::Mat bscanImg = cv::imdecode(bscanImageContent, cv::IMREAD_UNCHANGED);


			BScan::Data bscanData;

			// seglines

			BScan* bscan = new BScan(bscanImg, bscanData);
			readDataNode(bscanNode, *bscan);
			return bscan;
		}

		bool readBScanList(const bpt::ptree seriesNode, CppFW::UnzipCpp& zipfile, Series& series, CppFW::Callback* callback)
		{
			const std::size_t numBScan = seriesNode.count("BScan");
			CppFW::CallbackStepper bscanCallbackStepper(callback, numBScan);

			for(const std::pair<const std::string, bpt::ptree>& subTreePair : seriesNode)
			{
				if(subTreePair.first != "BScan")
					continue;

				if(++bscanCallbackStepper == false)
					return false;

				BScan* bscan = readBScan(subTreePair.second, zipfile);
				if(bscan)
					series.takeBScan(bscan);
			}
			return true;
		}

		template<typename S>
		std::string getSubStructureName()
		{
			std::string name = boost::typeindex::type_id<typename S::SubstructureType>().pretty_name();
			std::size_t namePos = name.rfind(':');
			if(namePos > 0)
				++namePos;
			return name.substr(namePos, name.size() - namePos);
		}



		// deep file format (support many scans per file, tree structure)
		template<typename S>
		bool readStructure(const bpt::ptree& tree, CppFW::UnzipCpp& zipfile, S& structure, CppFW::Callback* callback)
		{
			static const std::string subStructureName = getSubStructureName<S>();

			const std::size_t numSubStructure = tree.count(subStructureName);
			CppFW::CallbackStepper bscanCallbackStepper(callback, numSubStructure);

			bool result = true;
			readDataNode(tree, structure);

			for(const std::pair<const std::string, bpt::ptree>& subTreePair : tree)
			{
				if(subTreePair.first != subStructureName)
					continue;

				if(++bscanCallbackStepper == false)
					return false;

				const bpt::ptree& subTreeNode = subTreePair.second;
				const int id = subTreeNode.get<int>("id", 1);
				result &= readStructure(subTreeNode, zipfile, structure.getInsertId(id), callback);
			}
			return result;
		}


		template<>
		bool readStructure<Series>(const bpt::ptree& tree, CppFW::UnzipCpp& zipfile, Series& series, CppFW::Callback* callback)
		{
			readDataNode(tree, series);

			boost::optional<const bpt::ptree&> sloNode = tree.get_child_optional("slo");
			if(sloNode)
				series.takeSloImage(readSlo(*sloNode, zipfile));



// 			const CppFW::CVMatTree* sloNode = tree.getDirNodeOpt("slo");
//
// 			const CppFW::CVMatTree* bscansNode = getDirNodeOptCamelCase(tree, "bscans");
// 			if(!bscansNode)
// 				return false;
//
// 			const CppFW::CVMatTree::NodeList& seriesList = bscansNode->getNodeList();
			return readBScanList(tree, zipfile, series, callback);
// 			return true;
		}
	}

	XOctRead::XOctRead()
	: OctFileReader(OctExtension(".xoct", "XOct format"))
	{
	}

	bool OctData::XOctRead::readFile(OctData::FileReader& filereader, OctData::OCT& oct, const OctData::FileReadOptions& /*op*/, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();
		if(file.extension() != ".xoct")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as xoct";

		CppFW::UnzipCpp zipfile(file.generic_string());

		bpt::ptree xmlTree;
		std::vector<char> xmlRaw = zipfile.readFile("xoct.xml");
		bip::bufferstream input_stream(xmlRaw.data(), xmlRaw.size());
		bpt::read_xml(input_stream, xmlTree);

		if(callback)
			callback->callback(0.01);

		boost::optional<bpt::ptree&> xoctTree = xmlTree.get_child_optional("XOCT");

		if(xoctTree)
			readStructure(*xoctTree, zipfile, oct, callback);
		else
		{
			BOOST_LOG_TRIVIAL(error) << "XOCT node in xml not found";
			return false;
		}

/*
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
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" failed";*/

		return true;
	}

}
