/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
		enum class BScanType { Unknown, Line, Circle };

		typedef ObjectWrapper<BScanType> BScanTypeEnumWrapper;

		struct Data
		{
			std::string filename;


			int    numAverage              = 0;
			double imageQuality            = 0;
			double scanAngle               = 0;
			Date   acquisitionTime;
/*
			bool   positionWithinTolerance     ;
			bool   edi                         ;*/

			BScanType   bscanType          = BScanType::Line;
			ScaleFactor scaleFactor;
			CoordSLOmm  start      ;
			CoordSLOmm  end        ;
			CoordSLOmm  center     ;
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
		const cv::Mat& getAngioImage()      const                   { return *angioImage                 ; }
		const cv::Mat& getRawImage()        const                   { return *rawImage                   ; }

		void setRawImage(const cv::Mat& img);
		void setAngioImage(const cv::Mat& img);


		const std::string getFilename()     const                   { return data.filename               ; }

		int    getNumAverage()              const                   { return data.numAverage             ; }
		double getImageQuality()            const                   { return data.imageQuality           ; }
		double getScanAngle   ()            const                   { return data.scanAngle              ; }
		Date   getAcquisitionTime()         const                   { return data.acquisitionTime        ; }

// 		bool   getPositionWithinTolerance() const                   { return data.positionWithinTolerance; }
// 		bool   getEdi()                     const                   { return data.edi                    ; }

		      BScanType   getBScanType()    const                   { return data.bscanType              ; }
		const ScaleFactor& getScaleFactor() const                   { return data.scaleFactor            ; }
		const CoordSLOmm& getStart()        const                   { return data.start                  ; }
		const CoordSLOmm& getEnd()          const                   { return data.end                    ; }
		const CoordSLOmm& getCenter()       const                   { return data.center                 ; }
		      bool        getClockwiseRot() const                   { return data.clockwiseRotation      ; }

		const CoordSLOmm  getAscanPos(std::size_t ascan) const;
		const CoordSLOmm  getFracPos(double frac) const;


		constexpr static std::size_t getNumSegmentLine()            { return Segmentationlines::getSegmentlineTypes().size(); }
		const Segmentationlines::Segmentline& getSegmentLine(Segmentationlines::SegmentlineType i) const
		                                                            { return data.getSegmentLine(i); }

		const Segmentationlines& getSegmentLines() const            { return data.segmentationslines; }

		int   getWidth()                    const;
		int   getHeight()                   const;


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }


	private:
		cv::Mat*                                image      = nullptr;
		cv::Mat*                                angioImage = nullptr;
		cv::Mat*                                rawImage   = nullptr;
		Data                                    data;


		template<typename T, typename ParameterSet>
		static void callSubset(T& getSet, ParameterSet& p, const std::string& name)
		{
			T subSetGetSet = getSet.subSet(name);
			p.getSetParameter(subSetGetSet);
		}

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			BScanTypeEnumWrapper        bscanTypeWrapper       (p.data.bscanType);


			DateWrapper    acquisitionTimeWrapper(p.data.acquisitionTime);

			getSet("numAverage"       , p.data.numAverage                                );
			getSet("clockwiseRotation", p.data.clockwiseRotation                         );
			getSet("imageQuality"     , p.data.imageQuality                              );
			getSet("scanAngle"        , p.data.scanAngle                                 );
			getSet("acquisitionTime"  , static_cast<std::string&>(acquisitionTimeWrapper));
			getSet("bscanType"        , static_cast<std::string&>(bscanTypeWrapper)      );

			callSubset(getSet, p.data.scaleFactor, "scaleFactor");
			callSubset(getSet, p.data.start      , "start_mm"   );
			callSubset(getSet, p.data.end        , "end_mm"     );
			callSubset(getSet, p.data.center     , "center_mm"  );
		}

	};
}
