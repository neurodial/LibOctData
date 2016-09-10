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
		CoordTransform transform;

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

		const ScaleFactor&    getScaleFactor()         const        { return scaleFactor            ; }
		const CoordSLOpx&     getShift()               const        { return shift                  ; }
		const CoordTransform& getTransform()           const        { return transform              ; }
		void   setScaleFactor(const ScaleFactor& f)                 { scaleFactor = f               ; }
		void   setShift      (const CoordSLOpx&  s)                 { shift       = s               ; }
		void   setTransform  (const CoordTransform& t)              { transform   = t               ; }

		int    getNumAverage()                      const           { return numAverage             ; }
		int    getImageQuality()                    const           { return imageQuality           ; }
	};
}
