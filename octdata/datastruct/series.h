#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "date.h"
#include "analysegrid.h"

#include"objectwrapper.h"


#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif



namespace OctData
{
	class SloImage;
	class BScan;

	class Series
	{
		Series(const Series&)            = delete;
		Series& operator=(const Series&) = delete;

	public:
		enum class Laterality        { undef, OD, OS };
		enum class ScanPattern       { Unknown, Text, SingleLine, Circular, Volume, FastVolume, Radial, RadialCircles };
		enum class ExaminedStructure { Unknown, Text, ONH, Retina };
		typedef ObjectWrapper<Laterality       > LateralityEnumWrapper       ;
		typedef ObjectWrapper<ScanPattern      > ScanPatternEnumWrapper      ;
		typedef ObjectWrapper<ExaminedStructure> ExaminedStructureEnumWrapper;

		typedef std::vector<BScan*> BScanList;
		typedef std::vector<CoordSLOmm> BScanSLOCoordList;

		Octdata_EXPORTS explicit Series(int internalId);
		Octdata_EXPORTS ~Series();

		Octdata_EXPORTS const SloImage& getSloImage()            const { return *sloImage; }
		void takeSloImage(SloImage* sloImage);

		Octdata_EXPORTS const BScanList getBScans()              const { return bscans;    }
		Octdata_EXPORTS const BScan* getBScan(std::size_t pos) const;
		Octdata_EXPORTS std::size_t bscanCount()                 const { return bscans.size(); }

		Octdata_EXPORTS Laterality getLaterality()               const { return laterality; }
		void setLaterality(Laterality l)                               { laterality = l; }

		Octdata_EXPORTS ScanPattern getScanPattern()             const { return scanPattern; }
		Octdata_EXPORTS void setScanPattern(ScanPattern p)             { scanPattern = p;    }
		Octdata_EXPORTS const std::string& getScanPatternText()  const { return scanPatternText; }
		Octdata_EXPORTS void setScanPatternText(const std::string& str){ scanPatternText = str;  }

		Octdata_EXPORTS ExaminedStructure getExaminedStructure() const { return examinedStructure; }
		Octdata_EXPORTS void setExaminedStructure(ExaminedStructure p) { examinedStructure = p;    }
		Octdata_EXPORTS const std::string& getExaminedStructureText() const
		                                                               { return examinedStructureText; }
		Octdata_EXPORTS void setExaminedStructureText(const std::string& str)
		                                                               { examinedStructureText = str;  }

		void setScanDate(const Date& time)                             { scanDate = time; }
		Octdata_EXPORTS const Date& getScanDate()                const { return scanDate; }

		void setSeriesUID(const std::string& uid)                      { seriesUID = uid;  }
		Octdata_EXPORTS const std::string& getSeriesUID()        const { return seriesUID; }
		
		void setRefSeriesUID(const std::string& uid)                   { refSeriesID = uid;  }
		Octdata_EXPORTS const std::string& getRefSeriesUID()     const { return refSeriesID; }

		void setScanFocus(double focus)                                { scanFocus = focus; }
		Octdata_EXPORTS   double getScanFocus()                  const { return scanFocus;  }

		void takeBScan(BScan* bscan);

		void setDescription(const std::string& text)                   { description = text; }
		Octdata_EXPORTS const std::string& getDescription()      const { return description; }

		Octdata_EXPORTS int getInternalId()                      const { return internalId; }

		Octdata_EXPORTS       AnalyseGrid& getAnalyseGrid()            { return analyseGrid; }
		Octdata_EXPORTS const AnalyseGrid& getAnalyseGrid()      const { return analyseGrid; }

		Octdata_EXPORTS const BScanSLOCoordList& getConvexHull() const { return convexHullSLOBScans; }
		Octdata_EXPORTS const CoordSLOmm& getLeftUpperCoord()    const { return leftUpper; }
		Octdata_EXPORTS const CoordSLOmm& getRightLowerCoord()   const { return rightLower; }


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }

	private:
		const int internalId;

		SloImage*                               sloImage = nullptr;
		std::string                             seriesUID;
		std::string                             refSeriesID;
		double                                  scanFocus;

		ScanPattern                             scanPattern = ScanPattern::Unknown;
		std::string                             scanPatternText;

		ExaminedStructure                       examinedStructure = ExaminedStructure::Unknown;
		std::string                             examinedStructureText;

		Laterality                              laterality = Laterality::undef;
		Date                                    scanDate;

		std::string                             description;

		BScanList                               bscans;

		AnalyseGrid                             analyseGrid;

		BScanSLOCoordList                       convexHullSLOBScans;
		CoordSLOmm                              leftUpper;
		CoordSLOmm                              rightLower;
		void calculateSLOConvexHull();
		void updateCornerCoords();
		void updateCornerCoords(const CoordSLOmm& point);

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			LateralityEnumWrapper        lateralityWrapper       (p.laterality       );
			ScanPatternEnumWrapper       scanPatternWrapper      (p.scanPattern      );
			ExaminedStructureEnumWrapper examinedStructureWrapper(p.examinedStructure);


			DateWrapper    scanDateWrapper(p.scanDate);

			getSet("seriesUID"            , p.seriesUID                                        );
			getSet("refSeriesID"          , p.refSeriesID                                      );
			getSet("scanFocus"            , p.scanFocus                                        );
			getSet("scanPatternText"      , p.scanPatternText                                  );
			getSet("examinedStructureText", p.examinedStructureText                            );
			getSet("description"          , p.description                                      );
			getSet("scanDate"             , static_cast<std::string&>(scanDateWrapper         ));
			getSet("laterality"           , static_cast<std::string&>(lateralityWrapper       ));
			getSet("scanPattern"          , static_cast<std::string&>(scanPatternWrapper      ));
			getSet("examinedStructure"    , static_cast<std::string&>(examinedStructureWrapper));
		}

	};

}
