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
