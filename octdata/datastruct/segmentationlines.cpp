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

#include "segmentationlines.h"

namespace OctData
{

const Segmentationlines::SegLinesTypeList Segmentationlines::segmentlineTypes =
{
	{
		SegmentlineType::ILM ,
		SegmentlineType::RNFL,
		SegmentlineType::GCL ,
		SegmentlineType::IPL ,
		SegmentlineType::INL ,
		SegmentlineType::OPL ,
		SegmentlineType::ELM ,
		SegmentlineType::PR1 ,
		SegmentlineType::PR2 ,
		SegmentlineType::RPE ,
		SegmentlineType::BM  ,
		SegmentlineType::CHO
	}
};

// ILM, NFL, I3T1, I4T1, I5T1, I6T1, I8T3, I14T1, I15T1, I16T1, BM, NR_OF_ELEMENTS

const char* Segmentationlines::getSegmentlineName(OctData::Segmentationlines::SegmentlineType type)
{
	switch(type)
	{
		case SegmentlineType::ILM : return "ILM" ;
		case SegmentlineType::RNFL: return "RNFL";
		case SegmentlineType::GCL : return "GCL" ;
		case SegmentlineType::IPL : return "IPL" ;
		case SegmentlineType::INL : return "INL" ;
		case SegmentlineType::OPL : return "OPL" ;
		case SegmentlineType::ELM : return "ELM" ;
		case SegmentlineType::PR1 : return "PR1" ;
		case SegmentlineType::PR2 : return "PR2" ;
		case SegmentlineType::RPE : return "RPE" ;
		case SegmentlineType::BM  : return "BM"  ;
		case SegmentlineType::CHO : return "CHO" ;
	}
	return "invalid type";
}

}
