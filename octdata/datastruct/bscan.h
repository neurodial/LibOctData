#pragma once

#include <vector>
#include <array>
#include "coordslo.h"

namespace cv { class Mat; }

namespace OctData
{
	class BScan
	{
	public:
		enum class SegmentlineType{ ILM, BM, NR_OF_ELEMENTS };
		typedef std::vector<double> Segmentline;
		struct Data
		{
			std::string filename;

			ScaleFactor scaleFactor;

			int    numAverage              = 0 ;
			int    imageQuality            = 0 ;
/*
			bool   positionWithinTolerance     ;
			bool   edi                         ;*/

			CoordSLOmm start;
			CoordSLOmm end;
			CoordSLOmm center;

			static const std::size_t numSegmentlineType = static_cast<std::size_t>(SegmentlineType::NR_OF_ELEMENTS);
			std::array<Segmentline, numSegmentlineType> segmentlines;
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
		int    getImageQuality()            const                   { return data.imageQuality           ; }

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
