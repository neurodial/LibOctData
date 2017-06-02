#include "segmentationlines.h"

namespace OctData
{

const Segmentationlines::SegmentlineType Segmentationlines::segmentlineTypes[11] =
{
	SegmentlineType::ILM,
	SegmentlineType::NFL,
	SegmentlineType::I3T1,
	SegmentlineType::I4T1,
	SegmentlineType::I5T1,
	SegmentlineType::I6T1,
	SegmentlineType::I8T3,
	SegmentlineType::I14T1,
	SegmentlineType::I15T1,
	SegmentlineType::I16T1,
	SegmentlineType::BM
};

// ILM, NFL, I3T1, I4T1, I5T1, I6T1, I8T3, I14T1, I15T1, I16T1, BM, NR_OF_ELEMENTS

const char* Segmentationlines::getSegmentlineName(OctData::Segmentationlines::SegmentlineType type)
{
	switch(type)
	{
		case SegmentlineType::ILM:
			return "ILM";
		case SegmentlineType::NFL:
			return "NFL";
		case SegmentlineType::I3T1:
			return "I3T1";
		case SegmentlineType::I4T1:
			return "I4T1";
		case SegmentlineType::I5T1:
			return "I5T1";
		case SegmentlineType::I6T1:
			return "I6T1";
		case SegmentlineType::I8T3:
			return "I8T3";
		case SegmentlineType::I14T1:
			return "I14T1";
		case SegmentlineType::I15T1:
			return "I15T1";
		case SegmentlineType::I16T1:
			return "I16T1";
		case SegmentlineType::BM:
			return "BM";
	}
	return "invalid type";
}

}
