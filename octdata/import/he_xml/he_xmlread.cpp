#include "he_xmlread.h"



#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include <opencv2/opencv.hpp>

using boost::property_tree::ptree;
using boost::property_tree::xml_parser::read_xml;


/*
namespace
{
	std::string getFilename(ptree& imageNode)
	{
		boost::optional<ptree&> urlNode = imageNode.get_child_optional("ImageData.ExamURL");

		if(urlNode)
		{
			std::string urlFull = urlNode.get().get_value<std::string>();
			std::size_t filePos = urlFull.rfind('\\') + 1;
			return urlFull.substr(filePos);
		}
		std::cerr << "Image URL not found in XML\n";
		return std::string();
	}

	ScaleFactor readScaleFactor(ptree& dataNode)
	{
		double x = dataNode.get_child("ScaleX").get_value<double>();
		double y = dataNode.get_child("ScaleY").get_value<double>();

		// std::cout << "Scale: " << x << " | " << y << std::endl;

		return ScaleFactor(x, y);
	}

	CoordSLOmm readCoordmm(ptree& coordNode)
	{
		boost::optional<ptree&> xyCoordNode = coordNode.get_child_optional("Coord");

		if(xyCoordNode)
		{
			double x = xyCoordNode->get_child("X").get_value<double>();
			double y = xyCoordNode->get_child("Y").get_value<double>();

			// std::cout << x << " | " << y << std::endl;

			return CoordSLOmm(x, y);
		}
		return CoordSLOmm();
	}

	void fillSLOImage(ptree& imageNode, SLOImage* sloImage, const std::string& xmlPath)
	{
		sloImage->setFilename(getFilename(imageNode));


		sloImage->setScaleFactor(readScaleFactor(imageNode.get_child("OphthalmicAcquisitionContext")));

		std::string filepath = xmlPath + '/' + sloImage->getFilename();
		cv::Mat image = cv::imread(filepath, true);
		sloImage->setImage(image);
	}

	void fillBScann(ptree& imageNode, CScan& cscann, const std::string& xmlPath)
	{
		BScan::Data bscanData;


		std::string filename = getFilename(imageNode);
		std::string filepath = xmlPath + '/' + filename;
		cv::Mat image = cv::imread(filepath, true);
		
		
		boost::optional<ptree&> koordEndNode = imageNode.get_child_optional("OphthalmicAcquisitionContext.End");

		bscanData.start       = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Start"));
		if(koordEndNode)
			bscanData.end         = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.End"));
		else
			bscanData.center      = readCoordmm    (imageNode.get_child("OphthalmicAcquisitionContext.Center"));
			
		bscanData.scaleFactor = readScaleFactor(imageNode.get_child("OphthalmicAcquisitionContext"));


		cscann.takeBScan(new BScan(image, bscanData));
	}

}
*/


namespace OctData
{

	HeXmlRead::HeXmlRead()
	: OctFileReader(OctExtension("xml", "Heidelberg Engineering Xml File"))
	{
	}

	HeXmlRead* HeXmlRead::getInstance()
	{
		static HeXmlRead instance; return &instance;
	}

	bool HeXmlRead::readFile(const boost::filesystem::path& /*file*/, OCT& /*oct*/)
	{
		return false;
	}

}

/*

void OctXml::readOctXml(const std::string& filename, CScan* cscan)
{
	if(!cscan)
		return;

	bfs::path xmlfile(filename);
	if(!bfs::exists(xmlfile))
		return;
	// std::cout << xmlfile.branch_path() << std::endl;

	std::string xmlPath     = xmlfile.branch_path().generic_string();
	std::string xmlFilename = xmlfile.filename().generic_string();


	// Create an empty property tree object
	ptree pt;

	// Load the XML file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	read_xml(filename, pt);

/ *
	const char* numImageNodeStr = "HEDX.BODY.Patient.Study.Series.NumImages";

	boost::optional<ptree&> numImagesNode = pt.get_child(numImageNodeStr);
	if(!numImagesNode)
	{
		std::cerr << "no Node " << numImageNodeStr << std::endl;
		throw "";
	}
	std::size_t numImages = numImagesNode.get().get_value<std::size_t>();
* /

	for(std::pair<const std::string, ptree>& imageNode : pt.get_child("HEDX.BODY.Patient.Study.Series"))
	{
		if(imageNode.first != "Image")
			continue;

		// std::cout << t.first << std::endl;

		boost::optional<ptree&> type = imageNode.second.get_child_optional("ImageType.Type");

		if(!type)
		{
			std::cerr << __FILE__ << ":" << __LINE__ << ": image type not found\n";
			continue;
		}

		std::string typeStr = type.get().get_value<std::string>();

		if(typeStr == "LOCALIZER")
			fillSLOImage(imageNode.second, cscan->sloImage, xmlPath);

		if(typeStr == "OCT")
			fillBScann(imageNode.second, *cscan, xmlPath);
	}
}

*/



