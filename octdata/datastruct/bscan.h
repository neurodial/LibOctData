#pragma once

#include <vector>
#include <array>
#include "coordslo.h"
#include "date.h"
#include "segmentationlines.h"

namespace cv { class Mat; }


#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif


namespace OctData
{

	// GCL IPL INL OPL ELM PR1 PR2 RPE BM
	class Octdata_EXPORTS BScan
	{
	public:
		struct Data
		{
			std::string filename;

			ScaleFactor scaleFactor;

			int    numAverage              = 0 ;
			double imageQuality            = 0 ;
			Date   acquisitionTime;
/*
			bool   positionWithinTolerance     ;
			bool   edi                         ;*/

			CoordSLOmm start;
			CoordSLOmm end;
			CoordSLOmm center;
			bool clockwiseRotation = false;

			Segmentationlines segmentationslines;
			Segmentationlines::Segmentline& getSegmentLine(Segmentationlines::SegmentlineType i)
			                                                              { return segmentationslines.getSegmentLine(i); }
			const Segmentationlines::Segmentline& getSegmentLine(Segmentationlines::SegmentlineType i) const
			                                                              { return segmentationslines.getSegmentLine(i); }
		};

		// BScan();
		BScan(const cv::Mat& img, const BScan::Data& data);
		~BScan();

		BScan(const BScan& other)            = delete;
		BScan& operator=(const BScan& other) = delete;

		const cv::Mat& getImage()           const                   { return *image                      ; }
		const cv::Mat& getRawImage()        const                   { return *rawImage                   ; }

		void setRawImage(const cv::Mat& img);

		const std::string getFilename()     const                   { return data.filename               ; }

		int    getNumAverage()              const                   { return data.numAverage             ; }
		double getImageQuality()            const                   { return data.imageQuality           ; }
		Date   getAcquisitionTime()         const                   { return data.acquisitionTime        ; }

// 		bool   getPositionWithinTolerance() const                   { return data.positionWithinTolerance; }
// 		bool   getEdi()                     const                   { return data.edi                    ; }

		const ScaleFactor& getScaleFactor() const                   { return data.scaleFactor            ; }
		const CoordSLOmm& getStart()        const                   { return data.start                  ; }
		const CoordSLOmm& getEnd()          const                   { return data.end                    ; }
		const CoordSLOmm& getCenter()       const                   { return data.center                 ; }
		      bool        getClockwiseRot() const                   { return data.clockwiseRotation      ; }


		constexpr static std::size_t getNumSegmentLine()            { return Segmentationlines::getSegmentlineTypes().size(); }
		const Segmentationlines::Segmentline& getSegmentLine(Segmentationlines::SegmentlineType i) const
		                                                            { return data.getSegmentLine(i); }

		const Segmentationlines& getSegmentLines() const            { return data.segmentationslines; }

		int   getWidth()                    const;
		int   getHeight()                   const;

	private:
		cv::Mat*                                image    = nullptr;
		cv::Mat*                                rawImage = nullptr;
		Data                                    data;
	};
}
