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
		struct TreeImgGetter
		{
			const cv::Mat* get(const CppFW::CVMatTree* bscanNode)
			{
				const CppFW::CVMatTree* seriesImgNode = bscanNode->getDirNodeOpt("img");

				if(!seriesImgNode || seriesImgNode->type() != CppFW::CVMatTree::Type::Mat)
					return nullptr;

				return &(seriesImgNode->getMat());
			}
		};

		class OctDataImgGetter
		{
			const Series& refSeries;
			std::size_t pos = 0;

		public:
			OctDataImgGetter(const Series& refSeries) : refSeries(refSeries) {}

			const cv::Mat* get(const CppFW::CVMatTree* /*bscanNode*/)
			{
				if(refSeries.bscanCount() <= pos)
				{
					BOOST_LOG_TRIVIAL(warning) << "to less BScans in refFile: " << pos << " / " << refSeries.bscanCount();
					return nullptr;
				}
				const BScan* bscan = refSeries.getBScan(pos);
				++pos;
				if(!bscan)
					return nullptr;
				return &(bscan->getImage());
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
				if(!image)
					continue;


				BScan::Data bscanData;
				if(seriesSegNode)
				{
					const CppFW::CVMatTree* seriesSegILMNode = seriesSegNode->getDirNodeOpt("ILM");
					if(seriesSegILMNode && seriesSegILMNode->type() == CppFW::CVMatTree::Type::Mat)
					{
						const cv::Mat& segMat = seriesSegILMNode->getMat();

						if(segMat.type() == cv::DataType<double>::type)
						{
							const double* p = segMat.ptr<double>(0);
							std::vector<double> segVec(p, p + segMat.rows*segMat.cols);

							bscanData.segmentlines.at(static_cast<std::size_t>(BScan::SegmentlineType::ILM)) = segVec;
						}
					}
				}

				BScan* bscan = new BScan(*image, bscanData);
				series.takeBScan(bscan);
			}
			return true;
		}

		OCT tryOpenFile(const bfs::path& baseFile, const std::string& refFilename, const FileReadOptions& op, CppFW::Callback& callback)
		{
			bfs::path refFile(refFilename);
			bfs::path baseFilePath(baseFile.branch_path());

			// direkt path
			OCT octData = OctFileRead::openFile(refFilename, op, &callback);
			if(octData.size() > 0)
				return octData;

			// as rel path from base file
			bfs::path relPath = baseFilePath / refFile;
			octData = OctFileRead::openFile(relPath.generic_string(), op, &callback);
			if(octData.size() > 0)
				return octData;

			// only filename
			bfs::path filenamePath = baseFilePath / refFile.filename();
			octData = OctFileRead::openFile(filenamePath.generic_string(), op, &callback);

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
			return false;

		const CppFW::CVMatTree* patDataNode    = octtree.getDirNodeOpt("PatientData");
		const CppFW::CVMatTree* studyDataNode  = octtree.getDirNodeOpt("StudyData"  );
		const CppFW::CVMatTree* seriesDataNode = octtree.getDirNodeOpt("SeriesData" );

		int patId    = CppFW::CVMatTreeExtra::getCvScalar(patDataNode   , "ID", 1);
		int studyId  = CppFW::CVMatTreeExtra::getCvScalar(studyDataNode , "ID", 1);
		int seriesId = CppFW::CVMatTreeExtra::getCvScalar(seriesDataNode, "ID", 1);

		Patient& pat    = oct  .getPatient(patId   );
		Study&   study  = pat  .getStudy  (studyId );
		Series&  series = study.getSeries (seriesId);

		bool fillStatus = false;

		const CppFW::CVMatTree::NodeList& seriesList = seriesNode->getNodeList();

		bool internalImgData = true;
		const CppFW::CVMatTree* imgFileNameNode = octtree.getDirNodeOpt("imgFileName" );
		if(imgFileNameNode)
			internalImgData = (imgFileNameNode->type() != CppFW::CVMatTree::Type::String);


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
						}
					}
				}
			}

		}


		if(fillStatus)
			BOOST_LOG_TRIVIAL(debug) << "read bin file \"" << file.generic_string() << "\" finished";

		return fillStatus;
	}

	CvBinRead* CvBinRead::getInstance()
	{
		static CvBinRead instance;
		return &instance;
	}


}
