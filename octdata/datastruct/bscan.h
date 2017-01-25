#pragma once

#include <vector>
#include <array>
#include "coordslo.h"
#include "date.h"

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
		enum class SegmentlineType{ ILM, NFL, I3T1, I4T1, I5T1, I6T1, I8T3, I14T1, I15T1, I16T1, BM, NR_OF_ELEMENTS };
		typedef double SegmentlineDataType;
		typedef std::vector<SegmentlineDataType> Segmentline;
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

			static const std::size_t numSegmentlineType = static_cast<std::size_t>(SegmentlineType::NR_OF_ELEMENTS);
			std::array<Segmentline, numSegmentlineType> segmentlines;

			Segmentline& getSegmentLine(SegmentlineType i)  { return segmentlines.at(static_cast<std::size_t>(i)); }
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

		constexpr static std::size_t getNumSegmentLine()            { return Data::numSegmentlineType; }
		const Segmentline& getSegmentLine(SegmentlineType i) const  { return data.segmentlines.at(static_cast<std::size_t>(i)); }

//		void addSegLine(SegmentLine segLine)                        { seg.push_back(std::move(segLine)); }

		int   getWidth()                    const;
		int   getHeight()                   const;

	private:
		cv::Mat*                                image    = nullptr;
		cv::Mat*                                rawImage = nullptr;
		Data                                    data;

//		std::vector<SegmentLine>        seg;
	};
}
