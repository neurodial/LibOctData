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


#include <opencv2/opencv.hpp>


namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;


namespace OctData
{


	namespace
	{
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

		void fillBScann(const bpt::ptree& imageNode, Series& series, const std::string& xmlPath)
		{
			BScan::Data bscanData;


			std::string filename = getFilename(imageNode);
			std::string filepath = xmlPath + '/' + filename;
			cv::Mat image = cv::imread(filepath, true);


			boost::optional<const bpt::ptree&> koordEndNode = imageNode.get_child_optional("OphthalmicAcquisitionContext.End");

			bscanData.start       = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Start"));
			if(koordEndNode)
				bscanData.end         = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.End"));
			else
				bscanData.center      = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Center"));

			bscanData.scaleFactor = readScaleFactor(imageNode.get_child("OphthalmicAcquisitionContext"));


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

	bool HeXmlRead::readFile(const boost::filesystem::path& file, OCT& oct)
	{
		if(file.extension() != ".xml")
			return false;

		if(!bfs::exists(file))
			return false;
		// std::cout << xmlfile.branch_path() << std::endl;

		std::string xmlPath     = file.branch_path().generic_string();
		std::string xmlFilename = file.filename().generic_string();

		// Create an empty property tree object
		bpt::ptree pt;

		// Load the XML file into the property tree. If reading fails
		// (cannot open file, parse error), an exception is thrown.
		bpt::xml_parser::read_xml(file.generic_string(), pt);


		boost::optional<bpt::ptree&> hedxNode = pt.get_child_optional("HEDX");

		if(!hedxNode)
			return false; // no Heidelberg Engineering Xml File

		const char* patientNodeStr = "HEDX.BODY.Patient";


		bpt::ptree& patientNode = pt.get_child(patientNodeStr);


		std::string lastName       = patientNode.get_child("LastName"  ).get_value<std::string>();
		std::string firstNames     = patientNode.get_child("FirstNames").get_value<std::string>();
		std::string patientLongID  = patientNode.get_child("PatientID" ).get_value<std::string>();
		std::string sex            = patientNode.get_child("Sex"       ).get_value<std::string>();
		int         patientID      = patientNode.get_child("ID").get_value<int>();
		std::cout << xmlFilename << ": " << lastName << ", " << firstNames << std::endl;

		Patient& pat = oct.getPatient(patientID);
		pat.setForename(firstNames   );
		pat.setSurname (lastName     );
		pat.setId      (patientLongID);

		if(sex == "F")
			pat.setSex(Patient::Sex::Female);
		else if(sex == "M")
			pat.setSex(Patient::Sex::Male  );


		// copyPTree(pt, dest, "Series");


		bpt::ptree& studyNode = patientNode.get_child("Study");
		int         studyID   = studyNode  .get_child("ID"   ).get_value<int>();

		Study& study = pat.getStudy(studyID);
		for(const std::pair<const std::string, bpt::ptree>& seriesStudyPair : studyNode)
		{
			if(seriesStudyPair.first != "Series")
				continue;

			const bpt::ptree& seriesStudyNode = seriesStudyPair.second;

			int         seriesID          = seriesStudyNode.get_child("ID"               ).get_value<int>();
			std::string examinedStructure = seriesStudyNode.get_child("ExaminedStructure").get_value<std::string>();
			std::string type              = seriesStudyNode.get_child("Type"             ).get_value<std::string>();


			Series& series = study.getSeries(seriesID);
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
					fillSLOImage(imageNode.second, series, xmlPath);

				if(typeStr == "OCT")
					fillBScann(imageNode.second, series, xmlPath);

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




