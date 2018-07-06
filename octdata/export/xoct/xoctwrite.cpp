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

			template<typename T>
			void operator()(const std::string& name, const std::vector<T>& value)
			{
				std::ostringstream sstream;
				for(const T& val : value)
					sstream << val << ' ';
				tree.add(name, sstream.str());
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


		class XOctWritter
		{
			CppFW::ZipCpp& zipfile;
// 			const OctData::FileWriteOptions& opt;
			std::string imageExtention;
			bool compressImage = false;
		public:
			XOctWritter(CppFW::ZipCpp& zipfile
			          , const OctData::FileWriteOptions& opt)
			: zipfile(zipfile)
// 			, opt(opt)
			{
				switch(opt.xoctImageFormat)
				{
					case FileWriteOptions::XoctImageFormat::png:
						imageExtention = ".png";
						break;
					case FileWriteOptions::XoctImageFormat::tiff:
						imageExtention = ".tiff";
						break;
					case FileWriteOptions::XoctImageFormat::zippedBMP:
						compressImage = true;
						[[fallthrough]];
					case FileWriteOptions::XoctImageFormat::bmp:
						imageExtention = ".bmp";
						break;
				}
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


			void writeImage(bpt::ptree& node, const cv::Mat& image, const std::string& filename, const std::string& imageName)
			{
				if(image.empty())
					return;

				std::vector<uchar> buffer;
				cv::imencode(imageExtention, image, buffer);
				zipfile.addFile(filename, buffer.data(), buffer.size(), compressImage);
				node.add(imageName, filename);
			}

			void writeXml(const std::string& filename, bpt::ptree& xmlTree)
			{
				std::stringstream stream;
				bpt::write_xml(stream, xmlTree, bpt::xml_writer_make_settings<bpt::ptree::key_type>('\t', 1u));
				std::string xmlString = stream.str();
				zipfile.addFile(filename, xmlString.data(), static_cast<unsigned>(xmlString.size()));
			}


			template<typename S>
			void writeParameter(bpt::ptree& tree, const S& structure)
			{
				bpt::ptree& dataNode = tree.add("data", "");
				SetToPTree parameterWriter(dataNode);
				structure.getSetParameter(parameterWriter);
			}

			// general export methods
			void writeSlo(bpt::ptree& sloNode, const SloImage& slo, const std::string& dataPath)
			{
				writeParameter(sloNode, slo);
				writeImage(sloNode, slo.getImage(), dataPath + "slo" + imageExtention, "image");
			}


			void writeSegmentation(bpt::ptree& segNode, const Segmentationlines& seglines)
			{
				SetToPTree set(segNode);
				for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
				{
					const Segmentationlines::Segmentline& seg = seglines.getSegmentLine(type);
					if(!seg.empty())
						set(Segmentationlines::getSegmentlineName(type), seg);
				}
			}


			void writeBScan(bpt::ptree& seriesNode, const BScan* bscan, std::size_t bscanNum, const std::string& dataPath)
			{
				if(!bscan)
					return;

				std::string numString = boost::lexical_cast<std::string>(bscanNum);

				bpt::ptree& bscanNode = seriesNode.add("BScan", "");
				writeParameter(bscanNode, *bscan);
				writeImage(bscanNode, bscan->getImage()     , dataPath + "bscan_"      + numString + imageExtention, "image"     );
				writeImage(bscanNode, bscan->getAngioImage(), dataPath + "bscanAngio_" + numString + imageExtention, "angioImage");


				bpt::ptree seglinesTree;
				writeSegmentation(seglinesTree.add("LayerSegmentation", ""), bscan->getSegmentLines());

				std::string segmentationFile = dataPath + "segmentation_" + numString + ".xml";
				writeXml(segmentationFile, seglinesTree);
				bscanNode.add("LayerSegmentationFile", segmentationFile);
			}

			template<typename S>
			bool writeStructure(bpt::ptree& tree, const std::string& dataPath, const S& structure)
			{
				bool result = true;
				writeParameter(tree, structure);

				for(typename S::SubstructurePair const& subStructPair : structure)
				{
					std::string structureName = getSubStructureName<S>();
					bpt::ptree& subNode = tree.add(structureName, "");
					subNode.add("id", boost::lexical_cast<std::string>(subStructPair.first));

					std::string subDataPath = dataPath + structureName + '_' + boost::lexical_cast<std::string>(subStructPair.first) + '/';
					result &= writeStructure(subNode, subDataPath, *subStructPair.second);
				}
				return result;
			}

		};

		template<>
		bool XOctWritter::writeStructure<Series>(bpt::ptree& tree, const std::string& dataPath, const Series& series)
		{
			writeParameter(tree, series);
			writeSlo(tree.add("slo", ""), series.getSloImage(), dataPath);

			std::size_t bscanNum = 0;
			for(BScan* bscan : series.getBScans())
				writeBScan(tree, bscan, bscanNum++, dataPath);

			return true;
		}
	}



	bool XOctWrite::writeFile(const boost::filesystem::path& file, const OctData::OCT& oct, const OctData::FileWriteOptions& opt)
	{
		CppFW::ZipCpp zipfile(file.generic_string());

		bpt::ptree xmlTree;
		std::string dataPath;

		XOctWritter writter(zipfile, opt);

		bpt::ptree& octTree = xmlTree.add("XOCT", "");
		if(!writter.writeStructure(octTree, dataPath, oct))
			return false;

		writter.writeXml("xoct.xml", xmlTree);

		return true;
	}
}
