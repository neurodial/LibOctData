#include "cvbinoctwrite.h"

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
		void setCoords2Node(CppFW::CVMatTree& node, const CoordSLOmm& coords, const OctData::CoordSLOmm& shift)
		{
			double coordData[2] = { coords.getX()+shift.getX(), coords.getY()+shift.getY() };
			node.getMat() = cv::Mat(1, 2, cv::DataType<double>::type, coordData).clone();
		}
	}


	bool CvBinOctWrite::writeFile(const boost::filesystem::path& file, const OCT& oct)
	{

		CppFW::CVMatTree octtree;

		OCT::SubstructureCIterator pat = oct.begin();

		const int patId = pat->first;
		const Patient* p = pat->second;

		Patient::SubstructureCIterator study = p->begin();
		const Study* s = study->second;
		const int studyId = study->first;

		Study::SubstructureCIterator series = s->begin();
		const Series* ser = series->second;
		const int seriesId = series->first;


		CppFW::CVMatTree& patDataNode    = octtree.getDirNode("PatientData");
		CppFW::CVMatTree& studyDataNode  = octtree.getDirNode("StudyData"  );
		CppFW::CVMatTree& seriesDataNode = octtree.getDirNode("SeriesData" );

		CppFW::CVMatTreeExtra::setCvScalar(patDataNode   , "ID", patId   );
		CppFW::CVMatTreeExtra::setCvScalar(studyDataNode , "ID", studyId );
		CppFW::CVMatTreeExtra::setCvScalar(seriesDataNode, "ID", seriesId);


		const SloImage& slo = ser->getSloImage();
		octtree.getDirNode("slo").getMat() = slo.getImage();
		const CoordSLOpx& sloShift = slo.getShift();
		const CoordSLOmm& sloShiftMM = sloShift/slo.getScaleFactor();

		double sloScaleData[2] = { slo.getScaleFactor().getX(), slo.getScaleFactor().getY() };
		seriesDataNode.getDirNode("SloScale").getMat() = cv::Mat(1, 2, cv::DataType<double>::type, sloScaleData).clone();

		CppFW::CVMatTree& seriesNode = octtree.getDirNode("Serie");

		for(BScan* bscan : ser->getBScans())
		{
			if(!bscan)
				continue;

			CppFW::CVMatTree& bscanNode = seriesNode.newListNode();
			CppFW::CVMatTree& bscanImgNode = bscanNode.getDirNode("img");
			bscanImgNode.getMat() = bscan->getImage();

			CppFW::CVMatTree& bscanCoordNode = bscanNode.getDirNode("Coords");
			setCoords2Node(bscanCoordNode.getDirNode("Start"), bscan->getStart(), sloShiftMM);
			setCoords2Node(bscanCoordNode.getDirNode("End"  ), bscan->getEnd  (), sloShiftMM);


			CppFW::CVMatTree& bscanSegNode = bscanNode.getDirNode("Segmentations");
			const Segmentationlines::Segmentline& seg = bscan->getSegmentLine(Segmentationlines::SegmentlineType::ILM);
			bscanSegNode.getDirNode("ILM").getMat() = cv::Mat(1, static_cast<int>(seg.size()), cv::DataType<Segmentationlines::SegmentlineDataType>::type, const_cast<Segmentationlines::SegmentlineDataType*>(seg.data())).clone();

		}

		CppFW::CVMatTreeStructBin::writeBin(file.generic_string(), octtree);

		return true;
	}
}
