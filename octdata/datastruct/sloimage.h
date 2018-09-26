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

#include "coordslo.h"

namespace cv { class Mat; }



#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif

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

		template<typename T, typename ParameterSet>
		static void callSubset(T& getSet, ParameterSet& p, const std::string& name)
		{
			T subSetGetSet = getSet.subSet(name);
			p.getSetParameter(subSetGetSet);
		}

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("numAverage"  , p.numAverage  );
			getSet("imageQuality", p.imageQuality);

			callSubset(getSet, p.scaleFactor, "scaleFactor");
			callSubset(getSet, p.shift      , "shift_px"   );
			callSubset(getSet, p.transform  , "transform"  );
		}

	public:
		Octdata_EXPORTS SloImage();
		Octdata_EXPORTS ~SloImage();

		SloImage(const SloImage& other)            = delete;
		SloImage& operator=(const SloImage& other) = delete;

		const cv::Mat& getImage()                   const           { return *image                 ; }
		Octdata_EXPORTS void setImage(const cv::Mat& image);

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

		bool  hasImage()                            const           { return image                  ; }
		Octdata_EXPORTS int   getWidth()            const;
		Octdata_EXPORTS int   getHeight()           const;

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }
	};
}
