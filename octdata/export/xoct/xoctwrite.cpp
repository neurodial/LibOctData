#include "xoctwrite.h"

#include<fstream>


#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include<boost/property_tree/xml_parser.hpp>
#include <boost/type_index.hpp>

#include <opencv2/opencv.hpp>

#include <filewriteoptions.h>


#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>
#include <octfileread.h>

#include<cpp_framework/zip/zipcpp.h>

namespace bpt = boost::property_tree;



namespace OctData
{
	namespace
	{
		class SetToPTree
		{
			bpt::ptree& tree;
		public:
			SetToPTree(bpt::ptree& tree) : tree(tree) {}

			template<typename T>
			void operator()(const std::string& name, const T& value)
			{
				tree.add(name, value);
			}

			void operator()(const std::string& name, const std::string& value)
			{
				if(value.empty())
					return;
				tree.add(name, value);
			}

			SetToPTree subSet(const std::string& name)
			{
				return SetToPTree(tree.add(name, ""));
			}
		};



		void writeImage(bpt::ptree& node, CppFW::ZipCpp& zipfile, const cv::Mat& image, const std::string& filename, const std::string& imageName)
		{
			if(image.empty())
				return;

			std::vector<uchar> buffer;
			cv::imencode(".png", image, buffer);
			zipfile.addFile(filename, buffer.data(), buffer.size(), false);
			node.add(imageName, filename);
		}

		template<typename S>
		void writeParameter(bpt::ptree& tree, const S& structure)
		{
			bpt::ptree& dataNode = tree.add("data", "");
			SetToPTree parameterWriter(dataNode);
			structure.getSetParameter(parameterWriter);
		}

		// general export methods
		void writeSlo(bpt::ptree& sloNode, CppFW::ZipCpp& zipfile, const SloImage& slo, const std::string& dataPath)
		{
			writeParameter(sloNode, slo);
			writeImage(sloNode, zipfile, slo.getImage(), dataPath + "/slo.png", "image");
		}


		void writeBScan(bpt::ptree& seriesNode, CppFW::ZipCpp& zipfile, const BScan* bscan, std::size_t bscanNum, const std::string& dataPath)
		{
			if(!bscan)
				return;

			std::string numString = boost::lexical_cast<std::string>(bscanNum);

			bpt::ptree& bscanNode = seriesNode.add("BScan", "");
			writeParameter(bscanNode, *bscan);
			writeImage(bscanNode, zipfile, bscan->getImage()     , dataPath + "/bscan_"      + numString + ".png", "image"     );
			writeImage(bscanNode, zipfile, bscan->getAngioImage(), dataPath + "/bscanAngio_" + numString + ".png", "angioImage");


// 			CppFW::CVMatTree& bscanSegNode = bscanNode.getDirNode("segmentations");
//
// 			for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
// 			{
// 				const Segmentationlines::Segmentline& seg = bscan->getSegmentLine(type);
// 				if(!seg.empty())
// 					bscanSegNode.getDirNode(Segmentationlines::getSegmentlineName(type)).getMat() = cv::Mat(1, static_cast<int>(seg.size()), cv::DataType<Segmentationlines::SegmentlineDataType>::type, const_cast<Segmentationlines::SegmentlineDataType*>(seg.data())).clone();
// 			}
		}


		// deep file format (support many scans per file, tree structure)
		template<typename S>
		bool writeStructure(bpt::ptree& tree, CppFW::ZipCpp& zipfile, const std::string& dataPath, const S& structure)
		{
			bool result = true;
			writeParameter(tree, structure);

			for(typename S::SubstructurePair const& subStructPair : structure)
			{
				std::string name = boost::typeindex::type_id<typename S::SubstructureType>().pretty_name();
				std::size_t namePos = name.rfind(':');
				if(namePos > 0)
					++namePos;
				std::string structureName = name.substr(namePos, name.size() - namePos);
				bpt::ptree& subNode = tree.add(structureName, "");
				subNode.add("id", boost::lexical_cast<std::string>(subStructPair.first));

				std::string subDataPath = dataPath + '/' + structureName + '_' + boost::lexical_cast<std::string>(subStructPair.first);
				result &= writeStructure(subNode, zipfile, subDataPath, *subStructPair.second);
			}
			return result;
		}


		template<>
		bool writeStructure<Series>(bpt::ptree& tree, CppFW::ZipCpp& zipfile, const std::string& dataPath, const Series& series)
		{
			writeParameter(tree, series);
			writeSlo(tree.add("slo", ""), zipfile, series.getSloImage(), dataPath);

			std::size_t bscanNum = 0;
			for(BScan* bscan : series.getBScans())
				writeBScan(tree, zipfile, bscan, bscanNum++, dataPath);

			return true;
		}
	}



	bool XOctWrite::writeFile(const boost::filesystem::path& file, const OctData::OCT& oct, const OctData::FileWriteOptions& /*opt*/)
	{
		CppFW::ZipCpp zipfile(file.generic_string());

		bpt::ptree xmlTree;
		std::string dataPath;

		bpt::ptree& octTree = xmlTree.add("XOCT", "");
		if(!writeStructure(octTree, zipfile, dataPath, oct))
			return false;

		std::stringstream stream;
		bpt::write_xml(stream, xmlTree, bpt::xml_writer_make_settings<bpt::ptree::key_type>('\t', 1u));
		std::string xmlString = stream.str();
		zipfile.addFile("xoct.xml", xmlString.data(), static_cast<unsigned>(xmlString.size()));

		return true;
	}
}
