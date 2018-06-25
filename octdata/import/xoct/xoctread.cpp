#include "xoctread.h"

#include<locale>
#include<vector>
#include<sstream>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include<boost/property_tree/xml_parser.hpp>
#include<boost/interprocess/streams/bufferstream.hpp>
#include <boost/spirit/include/qi.hpp>

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
namespace qi = boost::spirit::qi;

namespace OctData
{
	namespace
	{

		class GetFromPTree
		{
			const bpt::ptree* tree;

			template<typename T>
			void unserializeVector(const std::string& str, std::vector<T>& vec)
			{
				std::istringstream sstream(str);
				T tmp;
				while(sstream.good())
				{
					sstream >> tmp;
					vec.push_back(tmp);
				}
			}

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

			template<typename T>
			void operator()(const std::string& name, std::vector<T>& value)
			{
				value.clear();
				if(tree)
				{
					boost::optional<std::string> t = tree->get_optional<std::string>(name);
					if(t)
						unserializeVector(*t, value);
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

		template<>
		void GetFromPTree::unserializeVector(const std::string& str, std::vector<double>& vec)
		{
			std::string::const_iterator f(str.begin()), l(str.end());
			qi::parse(f,l,qi::double_ % ' ',vec);
		}
// 		template<>
// 		void GetFromPTree::unserializeVector(const std::string& str, std::vector<float>& vec)
// 		{
// 			std::string::const_iterator f(str.begin()), l(str.end());
// 			qi::parse(f,l,qi::float_ % ' ',vec);
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

		bpt::ptree readXml(CppFW::UnzipCpp& zipfile, const std::string& filename)
		{
			bpt::ptree xmlTree;
			std::vector<char> xmlRaw = zipfile.readFile(filename);
			bip::bufferstream input_stream(xmlRaw.data(), xmlRaw.size());
			bpt::read_xml(input_stream, xmlTree);
			return xmlTree;
		}

		// general import methods
		cv::Mat readImage(const bpt::ptree& tree, CppFW::UnzipCpp& zipfile, const std::string& imageStr)
		{
			const boost::optional<std::string> imagePath = tree.get_optional<std::string>(imageStr);
			if(imagePath)
			{
				const std::vector<char> imageContent = zipfile.readFile(*imagePath);
				if(imageContent.size() > 0)
					return cv::imdecode(imageContent, cv::IMREAD_UNCHANGED);
			}
			return cv::Mat();
		}

		SloImage* readSlo(const bpt::ptree& sloNode, CppFW::UnzipCpp& zipfile)
		{
			cv::Mat sloImage = readImage(sloNode, zipfile, "image");
			if(sloImage.empty())
				return nullptr;

			SloImage* slo = new SloImage();
			slo->setImage(sloImage);
			readDataNode(sloNode, *slo);
			return slo;
		}

		void readSegmentation(const bpt::ptree& segNode, Segmentationlines& seglines)
		{
			GetFromPTree get(segNode);
			for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
			{
				Segmentationlines::Segmentline& seg = seglines.getSegmentLine(type);
				get(Segmentationlines::getSegmentlineName(type), seg);
			}
		}

		BScan* readBScan(const bpt::ptree& bscanNode, CppFW::UnzipCpp& zipfile)
		{
			cv::Mat bscanImg = readImage(bscanNode, zipfile, "image");
			if(bscanImg.empty())
				return nullptr;

			cv::Mat imageAngio = readImage(bscanNode, zipfile, "angioImage");

			BScan::Data bscanData;
			try// seglines
			{
				std::string layerSegmentationPath = bscanNode.get<std::string>("LayerSegmentationFile");
				bpt::ptree xmlTree = readXml(zipfile, layerSegmentationPath);
				bpt::ptree segNode = xmlTree.get_child("LayerSegmentation");
				readSegmentation(segNode, bscanData.segmentationslines);
			}
			catch(...) {}

			BScan* bscan = new BScan(bscanImg, bscanData);

			if(!imageAngio.empty())
				bscan->setAngioImage(imageAngio);

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


		template<typename S>
		bool readStructure(const bpt::ptree& tree, CppFW::UnzipCpp& zipfile, S& structure, const OctData::FileReadOptions& op, CppFW::Callback* callback)
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
				result &= readStructure(subTreeNode, zipfile, structure.getInsertId(id), op, callback);
			}
			return result;
		}


		template<>
		bool readStructure<Series>(const bpt::ptree& tree, CppFW::UnzipCpp& zipfile, Series& series, const OctData::FileReadOptions& op, CppFW::Callback* callback)
		{
			readDataNode(tree, series);

			boost::optional<const bpt::ptree&> sloNode = tree.get_child_optional("slo");
			if(sloNode)
				series.takeSloImage(readSlo(*sloNode, zipfile));

			if(op.readBScans)
				return readBScanList(tree, zipfile, series, callback);
			else
				return true;
		}
	}

	XOctRead::XOctRead()
	: OctFileReader(OctExtension(".xoct", "XOct format"))
	{
	}

	bool OctData::XOctRead::readFile(OctData::FileReader& filereader, OctData::OCT& oct, const OctData::FileReadOptions& op, CppFW::Callback* callback)
	{
		const boost::filesystem::path& file = filereader.getFilepath();
		if(file.extension() != ".xoct")
			return false;

		BOOST_LOG_TRIVIAL(trace) << "Try to open OCT file as xoct";

		CppFW::UnzipCpp zipfile(file.generic_string());

		bpt::ptree xmlTree = readXml(zipfile, "xoct.xml");

		if(callback)
			callback->callback(0.01);

		boost::optional<bpt::ptree&> xoctTree = xmlTree.get_child_optional("XOCT");

		if(xoctTree)
			readStructure(*xoctTree, zipfile, oct, op, callback);
		else
		{
			BOOST_LOG_TRIVIAL(error) << "XOCT node in xml not found";
			return false;
		}

		return true;
	}

}
