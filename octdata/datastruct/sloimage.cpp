#include "sloimage.h"


#include <opencv/cv.hpp>


namespace OctData
{
	SloImage::SloImage()
	: image(new cv::Mat)
	{

	}


	SloImage::~SloImage()
	{
		delete image;
	}


	void SloImage::setImage(const cv::Mat& image)
	{
		*(this->image) = image;
	}

}