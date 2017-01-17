#include "he_xmlread.h"



#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/series.h>
#include <datastruct/bscan.h>
#include <datastruct/oct.h>

#include <boost/filesystem.hpp>

#include "../platform_helper.h"

#include <opencv2/opencv.hpp>


#include <boost/log/trivial.hpp>


namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;


namespace OctData
{


	namespace
	{
		template<typename T>
		inline T readOptinalNode(const bpt::ptree& basis, const char* nodeName, T defaultValue)
		{
			boost::optional<const bpt::ptree&> optNode  = basis.get_child_optional(nodeName);
			if(optNode)
				return optNode->get_value<T>(defaultValue);
			return defaultValue;
		}


		std::string getFilename(const bpt::ptree& imageNode)
		{
			boost::optional<const bpt::ptree&> urlNode = imageNode.get_child_optional("ImageData.ExamURL");

			if(urlNode)
			{
				std::string urlFull = urlNode.get().get_value<std::string>();
				std::size_t filePos = urlFull.rfind('\\') + 1;
				return urlFull.substr(filePos);
			}
			std::cerr << "Image URL not found in XML\n";
			return std::string();
		}

		ScaleFactor readScaleFactor(const bpt::ptree& dataNode)
		{
			double x = dataNode.get_child("ScaleX").get_value<double>();
			double y = dataNode.get_child("ScaleY").get_value<double>();

			return ScaleFactor(x, y);
		}

		CoordSLOmm readCoordmm(const bpt::ptree& coordNode)
		{
			boost::optional<const bpt::ptree&> xyCoordNode = coordNode.get_child_optional("Coord");

			if(xyCoordNode)
			{
				double x = xyCoordNode->get_child("X").get_value<double>();
				double y = xyCoordNode->get_child("Y").get_value<double>();

				// std::cout << x << " | " << y << std::endl;

				return CoordSLOmm(x, y);
			}
			return CoordSLOmm();
		}

		void fillSLOImage(const bpt::ptree& imageNode, Series& series, const std::string& xmlPath)
		{
			SloImage* slo = new SloImage;

			slo->setScaleFactor(readScaleFactor(imageNode.get_child("OphthalmicAcquisitionContext")));
			std::string filepath = xmlPath + '/' + getFilename(imageNode);
			slo->setImage(cv::imread(filepath, true));
			series.takeSloImage(slo);
		}

		Date readDate(const bpt::ptree& dateNode)
		{
			int year  = readOptinalNode(dateNode, "Year" , 0);
			int month = readOptinalNode(dateNode, "Month", 0);
			int day   = readOptinalNode(dateNode, "Day"  , 0);

			return Date::fromDate(year, month, day);
		}

		Date readDateTime(const bpt::ptree& dateNode, const bpt::ptree& timeNode)
		{
			int    year  = readOptinalNode<int   >(dateNode, "Year"  , 0 );
			int    month = readOptinalNode<int   >(dateNode, "Month" , 0 );
			int    day   = readOptinalNode<int   >(dateNode, "Day"   , 0 );
			int    hour  = readOptinalNode<int   >(timeNode, "Hour"  , 0 );
			int    min   = readOptinalNode<int   >(timeNode, "Minute", 0 );
			double sec   = readOptinalNode<double>(timeNode, "Second", 0.);


			return Date::fromTime(year, month, day, hour, min, sec);
		}

		void fillStudy(const bpt::ptree& studyNode, Study& study)
		{
			study.setStudyUID     (studyNode.get_child("StudyUID"       ).get_value<std::string>(std::string()));
			study.setStudyOperator(studyNode.get_child("Operator"       ).get_value<std::string>(std::string()));


			boost::optional<const bpt::ptree&> studyDateNode = studyNode.get_child_optional("StudyDate.Date");
			if(studyDateNode)
				study.setStudyDate(readDate(*studyDateNode));
		}

		void fillSeriesLocalizer(const bpt::ptree& imageNode, Series& series)
		{
			boost::optional<const bpt::ptree&> scanFocusNode = imageNode.get_child_optional("OphthalmicAcquisitionContext.Focus");
			if(scanFocusNode)
				series.setScanFocus(scanFocusNode->get_value<double>(0.));
		}

		void fillSeries(const bpt::ptree& seriesNode, Series& series)
		{
			const std::string examinedStructure = readOptinalNode<std::string>(seriesNode, "ExaminedStructure", std::string());
			const std::string type              = readOptinalNode<std::string>(seriesNode, "Type"             , std::string());
			const std::string laterality        = readOptinalNode<std::string>(seriesNode, "Laterality"       , std::string());

			if(laterality == "R")
				series.setLaterality(Series::Laterality::OD);
			else if(laterality == "L")
				series.setLaterality(Series::Laterality::OS);

			if(!type.empty())
			{
				if(type == "Volume")
					series.setScanPattern(Series::ScanPattern::Volume);
				else
				{
					series.setScanPattern(Series::ScanPattern::Text);
					series.setScanPatternText(type);
				}
			}

			if(!examinedStructure.empty())
			{
				// TODO: parse text
				series.setExaminedStructure(Series::ExaminedStructure::Text);
				series.setExaminedStructureText(examinedStructure);
			}

			series.setSeriesUID   (readOptinalNode<std::string>(seriesNode,                 "SeriesUID", std::string()));
			series.setRefSeriesUID(readOptinalNode<std::string>(seriesNode, "ReferenceSeries.SeriesUID", std::string()));
		}

		void fillBScann(const bpt::ptree& imageNode, const bpt::ptree& studyNode, Series& series, const FileString& xmlPath)
		{
			BScan::Data bscanData;


			std::string filename = getFilename(imageNode);
			FileString filepath = xmlPath + '/' + filename;
			cv::Mat image = cv::imread(filepath, true); // TODO imdecode

			/// Separate the image in 3 places (B, Gand R)
			std::vector<cv::Mat> bgr_planes;
			cv::split(image, bgr_planes);
			image = bgr_planes[0];

			boost::optional<const bpt::ptree&> koordEndNode = imageNode.get_child_optional("OphthalmicAcquisitionContext.End");

			bscanData.start       = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Start"));
			if(koordEndNode)
				bscanData.end         = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.End"));
			else
				bscanData.center      = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Center"));

			bscanData.scaleFactor = readScaleFactor(imageNode.get_child("OphthalmicAcquisitionContext"));


			bscanData.numAverage   = imageNode.get_child("OphthalmicAcquisitionContext.NumAve").get_value<int>(0);
			bscanData.imageQuality = imageNode.get_child("OphthalmicAcquisitionContext.ImageQuality").get_value<int>(0);


			boost::optional<const bpt::ptree&> studyDateNode = studyNode.get_child_optional("StudyDate.Date");
			if(studyDateNode)
			{
				boost::optional<const bpt::ptree&> imageTimeNode = imageNode.get_child_optional("AcquisitionTime.Time");
				if(imageTimeNode)
					bscanData.acquisitionTime = readDateTime(*studyDateNode, *imageTimeNode);
			}

			series.takeBScan(new BScan(image, bscanData));

		}

	}



	HeXmlRead::HeXmlRead()
	: OctFileReader(OctExtension(".xml", "Heidelberg Engineering Xml File"))
	{
	}

	HeXmlRead* HeXmlRead::getInstance()
	{
		static HeXmlRead instance; return &instance;
	}

	bool HeXmlRead::readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& /*op*/)
	{
		if(file.extension() != ".xml")
			return false;


		BOOST_LOG_TRIVIAL(trace) << "Try to open Heidelberg Engineering Xml file as vol";

		FileString xmlPath     = filepathConv(file.branch_path());
		// std::string xmlFilename = file.filename().generic_string();

		// Create an empty property tree object
		bpt::ptree pt;


		std::fstream stream(filepathConv(file), std::ios::binary | std::ios::in);
		if(!stream.good())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open vol file " << filepathConv(file);
			return false;
		}

		// Load the XML file into the property tree. If reading fails
		// (cannot open file, parse error), an exception is thrown.
		bpt::xml_parser::read_xml(stream, pt);


		boost::optional<bpt::ptree&> hedxNode = pt.get_child_optional("HEDX");

		if(!hedxNode)
			return false; // no Heidelberg Engineering Xml File

		const char* patientNodeStr = "HEDX.BODY.Patient";


		bpt::ptree& patientNode = pt.get_child(patientNodeStr);

		
		boost::optional<bpt::ptree&> patUIDNode = patientNode.get_child_optional("PatientUIDList.PatientUID.UID");

		std::string lastName       = patientNode.get_child("LastName"  ).get_value<std::string>(std::string());
		std::string firstNames     = patientNode.get_child("FirstNames").get_value<std::string>(std::string());
		std::string patientLongID  = patientNode.get_child("PatientID" ).get_value<std::string>(std::string());
		std::string sex            = patientNode.get_child("Sex"       ).get_value<std::string>(std::string());
		int         patientID      = patientNode.get_child("ID"        ).get_value<int>(0);

		boost::optional<bpt::ptree&> patientBirthdateNode = patientNode.get_child_optional("Birthdate.Date");
		// std::cout << xmlFilename << ": " << lastName << ", " << firstNames << std::endl;

		Patient& pat = oct.getPatient(patientID);
		pat.setForename(firstNames   );
		pat.setSurname (lastName     );
		pat.setId      (patientLongID);
		if(patUIDNode)
			pat.setPatientUID(patUIDNode->get_value<std::string>(""));
		if(patientBirthdateNode)
			pat.setBirthdate(readDate(*patientBirthdateNode));

		if(sex == "F")
			pat.setSex(Patient::Sex::Female);
		else if(sex == "M")
			pat.setSex(Patient::Sex::Male  );

		// copyPTree(pt, dest, "Series");


		bpt::ptree& studyNode = patientNode.get_child("Study");
		int         studyID   = studyNode  .get_child("ID"   ).get_value<int>(0);

		Study& study = pat.getStudy(studyID);

		fillStudy(studyNode, study);

		for(const std::pair<const std::string, bpt::ptree>& seriesStudyPair : studyNode)
		{
			if(seriesStudyPair.first != "Series")
				continue;

			const bpt::ptree& seriesStudyNode = seriesStudyPair.second;

			int seriesID = seriesStudyNode.get_child("ID").get_value<int>(0);
			Series& series = study.getSeries(seriesID);
			
			fillSeries(seriesStudyNode, series);

			
			for(const std::pair<const std::string, bpt::ptree>& imageNode : seriesStudyNode)
			{
				if(imageNode.first != "Image")
					continue;


				boost::optional<const bpt::ptree&> type = imageNode.second.get_child_optional("ImageType.Type");

				if(!type)
				{
					std::cerr << __FILE__ << ":" << __LINE__ << ": image type not found\n";
					continue;
				}

				std::string typeStr = type.get().get_value<std::string>();

				if(typeStr == "LOCALIZER")
				{
					fillSLOImage(imageNode.second, series, xmlPath);
					fillSeriesLocalizer(imageNode.second, series);
				}

				if(typeStr == "OCT")
					fillBScann(imageNode.second, studyNode, series, xmlPath);

				/*
				boost::optional<const bpt::ptree&> urlNode = t.second.get_child_optional("ImageData.ExamURL");

				if(urlNode)
				{
					std::string urlFull   = urlNode.get().get_value<std::string>();
					std::size_t filePos   = urlFull.rfind('\\') + 1;
					bfs::path   imageName = urlFull.substr(filePos);

					bfs::path imageFile = xmlPath / imageName;
					if(bfs::exists(imageFile))
					{

					}
					else
						std::cout << "missing: " << imageFile.generic_string() << std::endl;
				}
				else
					std::cerr << "Image URL not found in XML\n";
				*/
			}
		}
		return true;
	}

}




