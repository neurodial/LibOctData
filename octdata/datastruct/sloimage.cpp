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

	int SloImage::getHeight() const
	{
		if(image)
			return image->rows;
		return 0;
	}

	int SloImage::getWidth() const
	{
		if(image)
			return image->cols;
		return 0;
	}


}
