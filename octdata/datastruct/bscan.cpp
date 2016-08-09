#include "bscan.h"


#include <opencv/cv.hpp>


namespace OctData
{

	BScan::BScan(const cv::Mat& img, const BScan::Data& data)
	: image   (new cv::Mat(img))
	, rawImage(new cv::Mat)
	, data    (data)
	{

	}

	BScan::~BScan()
	{
		delete image;
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

}