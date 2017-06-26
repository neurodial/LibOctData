#include "segmentationlines.h"

namespace OctData
{

const std::array<Segmentationlines::SegmentlineType, Segmentationlines::numSegmentlineType> Segmentationlines::segmentlineTypes =
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
