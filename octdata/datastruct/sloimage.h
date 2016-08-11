#pragma once

#include "coordslo.h"

namespace cv { class Mat; }

namespace OctData
{
	class SloImage
	{
		cv::Mat*    image    = nullptr;
		// std::string filename;

		ScaleFactor scaleFactor;
		CoordSLOpx  shift;

		int    numAverage              = 0 ;
		int    imageQuality            = 0 ;

	public:
		SloImage();
		~SloImage();

		SloImage(const SloImage& other)            = delete;
		SloImage& operator=(const SloImage& other) = delete;

		const cv::Mat& getImage()                   const           { return *image                 ; }
		void           setImage(const cv::Mat& image);

// 		const std::string& getFilename()             const          { return filename               ; }
// 		void               setFilename(const std::string& s)        {        filename = s           ; }

		const ScaleFactor& getScaleFactor()         const           { return scaleFactor            ; }
		const CoordSLOpx&  getShift()               const           { return shift                  ; }
		void               setScaleFactor(const ScaleFactor& f)     { scaleFactor = f               ; }
		void               setShift      (const CoordSLOpx&  s)     { shift       = s               ; }

		int    getNumAverage()                      const           { return numAverage             ; }
		int    getImageQuality()                    const           { return imageQuality           ; }
	};
}
