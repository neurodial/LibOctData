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
#include <cpp_framework/cvmat/treestructbin.h>

namespace OctData
{

	CvBinRead::CvBinRead()
	: OctFileReader(OctExtension(".bin", "CvBin format"))
	{
	}

	bool CvBinRead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& /*op*/)
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
			return false;


		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1);

		const CppFW::CVMatTree::NodeList& seriesList = seriesNode->getNodeList();

		for(const CppFW::CVMatTree* bscanNode : seriesList)
		{
			if(!bscanNode)
				continue;

			const CppFW::CVMatTree* seriesImgNode = bscanNode->getDirNodeOpt("img");
			const CppFW::CVMatTree* seriesSegNode = bscanNode->getDirNodeOpt("Segmentations");

			if(!seriesImgNode || seriesImgNode->type() != CppFW::CVMatTree::Type::Mat)
				continue;

			const cv::Mat& image = seriesImgNode->getMat();

			BScan::Data bscanData;
			if(seriesSegNode)
			{
				const CppFW::CVMatTree* seriesSegILMNode = seriesSegNode->getDirNodeOpt("ILM");
				if(seriesSegILMNode && seriesSegILMNode->type() == CppFW::CVMatTree::Type::Mat)
				{
					const cv::Mat& segMat = seriesSegILMNode->getMat();

					const double* p = segMat.ptr<double>(0);
					std::vector<double> segVec(p, p + segMat.rows*segMat.cols);

					bscanData.segmentlines.at(static_cast<std::size_t>(BScan::SegmentlineType::ILM)) = segVec;
				}
			}

			BScan* bscan = new BScan(image, bscanData);
			series.takeBScan(bscan);
		}


		BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" finished";
		return true;
	}

	CvBinRead* CvBinRead::getInstance()
	{
		static CvBinRead instance;
		return &instance;
	}


}
