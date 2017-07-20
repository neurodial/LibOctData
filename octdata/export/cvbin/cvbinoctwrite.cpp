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


		const OctData::SloImage& slo = ser->getSloImage();
		octtree.getDirNode("slo").getMat() = slo.getImage();


		CppFW::CVMatTree& seriesNode = octtree.getDirNode("Serie");

		for(BScan* bscan : ser->getBScans())
		{
			if(!bscan)
				continue;

			CppFW::CVMatTree& bscanNode = seriesNode.newListNode();
			CppFW::CVMatTree& bscanImgNode = bscanNode.getDirNode("img");
			bscanImgNode.getMat() = bscan->getImage();
		}


/*


		if(!seriesNode || seriesNode->type() != CppFW::CVMatTree::Type::List)
		{
			BOOST_LOG_TRIVIAL(trace) << "Serie node not found or false datatype";
			return false;
		}
		Patient& pat    = oct  .getPatient(patId   );
		Study&   study  = pat  .getStudy  (studyId );
		Series&  series = study.getSeries (seriesId);

		series.setRefSeriesUID(CppFW::CVMatTreeExtra::getStringOrEmpty(seriesDataNode, "RefUID"));
		series.setSeriesUID   (CppFW::CVMatTreeExtra::getStringOrEmpty(seriesDataNode, "UID"   ));
		pat   .setId          (CppFW::CVMatTreeExtra::getStringOrEmpty(patDataNode   , "PatID" ));

*/


		CppFW::CVMatTreeStructBin::writeBin(file.generic_string(), octtree);

		return true;
	}
}
