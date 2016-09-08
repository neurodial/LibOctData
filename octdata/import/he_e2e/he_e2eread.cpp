#include "he_e2eread.h"

#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>
#include <filereadoptions.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>

#include <E2E/e2edata.h>
#include <E2E/dataelements/patientdataelement.h>
#include <E2E/dataelements/image.h>
#include <E2E/dataelements/segmentationdata.h>
#include <E2E/dataelements/bscanmetadataelement.h>
#include <E2E/dataelements/imageregistration.h>

#include "he_gray_transform.h"


namespace bfs = boost::filesystem;


namespace OctData
{

	namespace
	{
		void copyPatData(Patient& pat, const E2E::Patient& e2ePat)
		{
			const E2E::PatientDataElement* e2ePatData = e2ePat.getPatientData();
			if(!e2ePatData)
				return;

			pat.setForename(e2ePatData->getForename());
			pat.setSurname (e2ePatData->getSurname ());
			pat.setId      (e2ePatData->getId      ());
			// pat.setSex     (e2ePatData.getSex     ());
			pat.setTitle   (e2ePatData->getTitle   ());
		}

		void copySlo(Series& series, const E2E::Series& e2eSeries)
		{
			const E2E::Image* e2eSlo = e2eSeries.getSloImage();
			if(!e2eSlo)
				return;

			SloImage* slo = new SloImage;

			slo->setShift(CoordSLOpx(e2eSlo->getImageRows()/2., e2eSlo->getImageCols()/2.));
			slo->setImage(e2eSlo->getImage());
			series.takeSloImage(slo);
		}

		void addSegData(BScan::Data& bscanData, BScan::SegmentlineType segType, const E2E::BScan::SegmentationMap& e2eSegMap, std::size_t index, std::size_t type, const E2E::ImageRegistration* reg, std::size_t imagecols)
		{
			const E2E::BScan::SegmentationMap::const_iterator segPair = e2eSegMap.find(E2E::BScan::SegPair(index, type));
			if(segPair != e2eSegMap.end())
			{
				E2E::SegmentationData* segData = segPair->second;
				if(segData)
				{
					typedef std::vector<double> SegDataVec;
					std::size_t numSegData = segData->size();
					SegDataVec segVec(numSegData);
					if(reg)
					{
						double shiftY = -reg->values[3];
						double degree = -reg->values[7];
						double shiftX = -reg->values[9] - degree*imagecols/2.;
						int    shiftXVec = std::round(shiftY);
						double pos    = shiftXVec;

						SegDataVec::iterator segVecBegin = segVec.begin();
						E2E::SegmentationData::pointer segDataBegin = segData->begin();

						int numAssign = numSegData-abs(shiftXVec);

						if(shiftXVec < 0)
							segDataBegin -= shiftXVec;
						if(shiftXVec > 0)
							segVecBegin += shiftXVec;

						std::transform(segDataBegin, segDataBegin+numAssign, segVecBegin, [&pos, shiftX, degree](double value) { return value + shiftX + (++pos)*degree; });
					}
					else
						segVec.assign(segData->begin(), segData->end());

					bscanData.segmentlines.at(static_cast<std::size_t>(segType)) = std::move(segVec);
				}
			}
		}

		template<typename T>
		void fillEmptyBroderCols(cv::Mat& image, T broderValue, T fillValue)
		{
			const int cols = image.cols;
			const int rows = image.rows;

			// find left Broder
			int leftEnd = cols;
			for(int row = 0; row<rows; ++row)
			{
				T* it = image.ptr<T>(row);
				for(int col = 0; col < leftEnd; ++col)
				{
					if(*it != broderValue)
					{
						leftEnd = col;
						break;
					}
					++it;
				}
				if(leftEnd == 0)
					break;
			}

			// fill left Broder
			if(leftEnd > 0)
			{
				for(int row = 0; row<rows; ++row)
				{
					T* it = image.ptr<T>(row);
					for(int col = 0; col < leftEnd; ++col)
					{
						*it = fillValue;
						++it;
					}
				}
			}
			else
				if(leftEnd == cols) // empty Image
					return;

			// find right Broder
			int rightEnd = leftEnd;
			for(int row = 0; row<rows; ++row)
			{
				T* it = image.ptr<T>(row, cols-1);
				for(int col = cols-1; col >= rightEnd; --col)
				{
					if(*it != broderValue)
					{
						rightEnd = col;
						break;
					}
					--it;
				}
				if(rightEnd == cols-1)
					break;
			}

			// fill right Broder
			if(rightEnd < cols)
			{
				for(int row = 0; row<rows; ++row)
				{
					T* it = image.ptr<T>(row, rightEnd);
					for(int col = rightEnd; col < cols; ++col)
					{
						*it = fillValue;
						++it;
					}
				}
			}
		}
		
		template<typename SourceType, typename DestType, typename TransformType>
		void useLUTBScan(const cv::Mat& source, cv::Mat& dest)
		{
			dest.create(source.rows, source.cols, cv::DataType<DestType>::type);

			TransformType& lut = TransformType::getInstance();
			
			const SourceType* sPtr = source.ptr<SourceType>();
			DestType* dPtr = dest.ptr<DestType>();
			
			const std::size_t size = source.cols * source.rows;
			for(size_t i = 0; i < size; ++i)
			{
				*dPtr = lut.getValue(*sPtr);
				++dPtr;
				++sPtr;
			}
		}

		void copyBScan(Series& series, const E2E::BScan& e2eBScan, const FileReadOptions& op)
		{
			const E2E::Image* e2eBScanImg = e2eBScan.getImage();
			if(!e2eBScanImg)
				return;

			const cv::Mat& e2eImage = e2eBScanImg->getImage();


			BScan::Data bscanData;

			const E2E::BScanMetaDataElement* e2eMeta = e2eBScan.getBScanMetaDataElement();
			if(e2eMeta)
			{
				uint32_t factor = 30; // TODO
				bscanData.start = CoordSLOmm(e2eMeta->getX1()*factor, e2eMeta->getY1()*factor);
				bscanData.end   = CoordSLOmm(e2eMeta->getX2()*factor, e2eMeta->getY2()*factor);
			}

			const E2E::ImageRegistration* reg = nullptr;
			if(op.registerBScanns)
				reg = e2eBScan.getImageRegistrationData();

			// segmenation lines
			const E2E::BScan::SegmentationMap& e2eSegMap = e2eBScan.getSegmentationMap();
			addSegData(bscanData, BScan::SegmentlineType::ILM, e2eSegMap, 0, 5, reg, e2eImage.cols);
			addSegData(bscanData, BScan::SegmentlineType::BM , e2eSegMap, 1, 2, reg, e2eImage.cols);
			addSegData(bscanData, BScan::SegmentlineType::NFL, e2eSegMap, 2, 7, reg, e2eImage.cols);

			// convert image
			cv::Mat dest, bscanImageConv;
			switch(op.e2eGray)
			{
			case FileReadOptions::E2eGrayTransform::nativ:
				e2eImage.convertTo(dest, CV_32FC1, 1/static_cast<double>(1 << 16), 0);
				cv::pow(dest, 8, dest);
				dest.convertTo(bscanImageConv, CV_8U, 255, 0);
				break;
			case FileReadOptions::E2eGrayTransform::xml:
				useLUTBScan<uint16_t, uint8_t, HeGrayTransformXml>(e2eImage, bscanImageConv);
				break;
			case FileReadOptions::E2eGrayTransform::vol:
				useLUTBScan<uint16_t, uint8_t, HeGrayTransformVol>(e2eImage, bscanImageConv);
				break;
			}

			if(!op.fillEmptyPixelWhite)
				fillEmptyBroderCols<uint8_t>(bscanImageConv, 255, 0);

			// testcode
			if(reg)
			{
				// std::cout << "shift X: " << reg->values[9] << std::endl;
				double shiftY = -reg->values[3];
				double degree = -reg->values[7];
				double shiftX = -reg->values[9];
				// std::cout << "shift X: " << shiftX << "\tdegree: " << degree << "\t" << (degree*bscanImageConv.cols/2) << std::endl;
				cv::Mat trans_mat = (cv::Mat_<double>(2,3) << 1, 0, shiftY, degree, 1, shiftX - degree*bscanImageConv.cols/2.);

				uint8_t fillValue = 0;
				if(op.fillEmptyPixelWhite)
					fillValue = 255;
				cv::warpAffine(bscanImageConv, bscanImageConv, trans_mat, bscanImageConv.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(fillValue));
			}

			BScan* bscan = new BScan(bscanImageConv, bscanData);
			bscan->setRawImage(e2eImage);
			series.takeBScan(bscan);
		}
	}


	HeE2ERead::HeE2ERead()
	: OctFileReader({OctExtension(".E2E", "Heidelberg Engineering E2E File"), OctExtension(".sdb", "Heidelberg Engineering HEYEX File")})
	{
	}

	bool HeE2ERead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& op)
	{
		if(file.extension() != ".E2E" && file.extension() != ".sdb")
			return false;

		if(!bfs::exists(file))
			return false;

		E2E::E2EData e2eData;
		e2eData.readE2EFile(file.generic_string());

		const E2E::DataRoot& e2eRoot = e2eData.getDataRoot();


		// std::vector<int> loadedPatients;
		std::vector<int> loadedStudies;
		std::vector<int> loadedSeries;

		for(const E2E::DataRoot::SubstructurePair& e2ePatPair : e2eRoot)
		{
			for(const E2E::Patient::SubstructurePair& e2eStudyPair : *(e2ePatPair.second))
			{
				loadedStudies.push_back(e2eStudyPair.first);

				for(const E2E::Study::SubstructurePair& e2eSeriesPair : *(e2eStudyPair.second))
				{
					loadedSeries.push_back(e2eSeriesPair.first);
				}
			}
		}

		// load extra Data from patient file (pdb) and study file (edb)
		if(file.extension() == ".sdb")
		{
			for(const E2E::DataRoot::SubstructurePair& e2ePatPair : e2eRoot)
			{
				char buffer[100];
				const E2E::Patient& e2ePat = *(e2ePatPair.second);
				std::sprintf(buffer, "%08d.pdb", e2ePatPair.first);
				// std::string filenname =
				bfs::path patientDataFile(file.branch_path() / buffer);
				if(bfs::exists(patientDataFile))
					e2eData.readE2EFile(patientDataFile.generic_string());

				for(const E2E::Patient::SubstructurePair& e2eStudyPair : e2ePat)
				{
					std::sprintf(buffer, "%08d.edb", e2eStudyPair.first);
					bfs::path studyDataFile(file.branch_path() / buffer);
					if(bfs::exists(studyDataFile))
						e2eData.readE2EFile(studyDataFile.generic_string());
				}
			}
		}

		// convert e2e structure in octdata structure
		for(const E2E::DataRoot::SubstructurePair& e2ePatPair : e2eRoot)
		{
			Patient& pat = oct.getPatient(e2ePatPair.first);
			const E2E::Patient& e2ePat = *(e2ePatPair.second);
			copyPatData(pat, e2ePat);

		//	for(const E2E::Patient::SubstructurePair& e2eStudyPair : e2ePat)
		//	{
		//		Study& study = pat.getStudy(e2eStudyPair.first);
		//		const E2E::Study& e2eStudy = *(e2eStudyPair.second);
			for(int studyID : loadedStudies)
			{
				std::cout << "studyID: " << studyID << std::endl;
				Study& study = pat.getStudy(studyID);
				const E2E::Study& e2eStudy = e2ePat.getStudy(studyID);


			//	for(const E2E::Study::SubstructurePair& e2eSeriesPair : e2eStudy)
			//	{
			//		Series& series = study.getSeries(e2eSeriesPair.first);
			//		const E2E::Series& e2eSeries = *(e2eSeriesPair.second);

				for(int seriesID : loadedSeries)
				{
					std::cout << "seriesID: " << seriesID << std::endl;
					Series& series = study.getSeries(seriesID);
					const E2E::Series& e2eSeries = e2eStudy.getSeries(seriesID);
					copySlo(series, e2eSeries);


					for(const E2E::Series::SubstructurePair& e2eBScanPair : e2eSeries)
					{
						copyBScan(series, *(e2eBScanPair.second), op);
					}
				}
			}
		}

		return true;
	}

	HeE2ERead* HeE2ERead::getInstance()
	{
		static HeE2ERead instance; return &instance;
	}


}
