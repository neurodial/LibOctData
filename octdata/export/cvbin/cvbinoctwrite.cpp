#include "cvbinoctwrite.h"

#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <filereadoptions.h>


#include <boost/log/trivial.hpp>

#include <cpp_framework/cvmat/cvmattreestruct.h>
#include <cpp_framework/cvmat/cvmattreestructextra.h>
#include <cpp_framework/cvmat/cvmattreegetset.h>
#include <cpp_framework/cvmat/treestructbin.h>
#include <cpp_framework/callback.h>
#include <octfileread.h>


namespace bfs = boost::filesystem;


namespace OctData
{
	namespace
	{
		void writeSlo(CppFW::CVMatTree& sloNode, const SloImage& slo)
		{
			CppFW::SetToCVMatTree sloWriter(sloNode);
			slo.getSetParameter(sloWriter);
			sloNode.getDirNode("img").getMat() = slo.getImage();
		}


		template<typename S>
		bool writeStructure(CppFW::CVMatTree& tree, const S& structure)
		{
			bool result = true;
			CppFW::CVMatTree& dataNode    = tree.getDirNode("data");
// 			CppFW::CVMatTree& subNodeList = tree.getDirNode("Sub"); // TODO
			CppFW::SetToCVMatTree structureWriter(dataNode);
			structure.getSetParameter(structureWriter);

			for(typename S::SubstructurePair const& subStructPair : structure)
			{
				CppFW::CVMatTree& subNode = tree.getDirNode("id_" + boost::lexical_cast<std::string>(subStructPair.first));
				result &= writeStructure(subNode, *subStructPair.second);
			}
			return result;
		}

		template<>
		bool writeStructure<Series>(CppFW::CVMatTree& tree, const Series& series)
		{
			CppFW::CVMatTree& seriesDataNode = tree.getDirNode("data");
			CppFW::SetToCVMatTree seriesWriter(seriesDataNode);
			series.getSetParameter(seriesWriter);


			writeSlo(tree.getDirNode("slo"), series.getSloImage());

			CppFW::CVMatTree& seriesNode = tree.getDirNode("bscans");

			for(BScan* bscan : series.getBScans())
			{
				if(!bscan)
					continue;

				CppFW::CVMatTree& bscanNode = seriesNode.newListNode();
				CppFW::CVMatTree& bscanImgNode = bscanNode.getDirNode("img");
				bscanImgNode.getMat() = bscan->getImage();


				CppFW::CVMatTree& bscanDataNode = bscanNode.getDirNode("data");
				CppFW::SetToCVMatTree bscanWriter(bscanDataNode);
				bscan->getSetParameter(bscanWriter);


				CppFW::CVMatTree& bscanSegNode = bscanNode.getDirNode("Segmentations");


				for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
				{
					const Segmentationlines::Segmentline& seg = bscan->getSegmentLine(type);
					if(!seg.empty())
						bscanSegNode.getDirNode(Segmentationlines::getSegmentlineName(type)).getMat() = cv::Mat(1, static_cast<int>(seg.size()), cv::DataType<Segmentationlines::SegmentlineDataType>::type, const_cast<Segmentationlines::SegmentlineDataType*>(seg.data())).clone();
				}
			}

			return true;
		}

	}

	void setStringOpt(CppFW::CVMatTree& tree, const char* nodeName, const std::string& value)
	{
		if(!value.empty())
			tree.getDirNode(nodeName).getString() = value;
	}

	bool CvBinOctWrite::writeFile(const boost::filesystem::path& file, const OCT& oct)
	{
		CppFW::CVMatTree octtree;

		bool result = writeStructure(octtree, oct);

// 		if(result)
			CppFW::CVMatTreeStructBin::writeBin(file.generic_string(), octtree);

		return result;
	}
}
