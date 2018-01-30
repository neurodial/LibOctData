#include "bscan.h"


#include <opencv/cv.hpp>


namespace OctData
{

	BScan::BScan(const cv::Mat& img, const BScan::Data& data)
	: image     (new cv::Mat(img))
	, angioImage(new cv::Mat)
	, rawImage  (new cv::Mat)
	, data      (data)
	{

	}

	BScan::~BScan()
	{
		delete image;
		delete angioImage;
		delete rawImage;
	}

	int BScan::getWidth() const
	{
		return image->cols;
	}

	int BScan::getHeight() const
	{
		return image->rows;
	}

	void BScan::setRawImage(const cv::Mat& img)
	{
		*rawImage = img;
	}

	void BScan::setAngioImage(const cv::Mat& img)
	{
		*angioImage = img;
	}


	template<> void BScan::BScanTypeEnumWrapper::toString()
	{
		switch(obj)
		{
			case BScan::BScanType::Circle: std::string::operator=("Circle"); break;
			case BScan::BScanType::Line  : std::string::operator=("Line"  ); break;
			case BScan::BScanType::Unknown:
				break;
		}
	}
	template<> void BScan::BScanTypeEnumWrapper::fromString()
	{
		     if(*this == "Circle") obj = BScan::BScanType::Circle;
		else if(*this == "Line"  ) obj = BScan::BScanType::Line  ;
		else obj = BScan::BScanType::Unknown;
	}
}
