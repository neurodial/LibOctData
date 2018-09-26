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

#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif


namespace OctData
{

	// GCL IPL INL OPL ELM PR1 PR2 RPE BM
	class Octdata_EXPORTS  Segmentationlines
	{
		static const std::size_t numSegmentlineType = 12;
	public:
		enum class SegmentlineType
		{
			ILM ,
			RNFL,
			GCL ,
			IPL ,
			INL ,
			OPL ,
			ELM ,
			PR1 ,
			PR2 ,
			RPE ,
			BM  ,
			CHO
		};

		typedef double SegmentlineDataType;
		typedef std::vector<SegmentlineDataType> Segmentline;
		typedef std::array<SegmentlineType, numSegmentlineType> SegLinesTypeList;

		      Segmentline& getSegmentLine(SegmentlineType i)           { return segmentlines.at(static_cast<std::size_t>(i)); }
		const Segmentline& getSegmentLine(SegmentlineType i)     const { return segmentlines.at(static_cast<std::size_t>(i)); }

		static const char* getSegmentlineName(SegmentlineType type);

		constexpr static const SegLinesTypeList& getSegmentlineTypes() { return segmentlineTypes; }
	private:
		static const SegLinesTypeList segmentlineTypes;
		std::array<Segmentline, numSegmentlineType> segmentlines;

	};

}
