#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "date.h"



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
		enum class Laterality { undef, OD, OS };
		enum class ScanPattern { Unknown, SingleLine, Circular, Volume, FastVolume, Radial };
		typedef std::vector<BScan*> BScanList;

		Octdata_EXPORTS explicit Series(int internalId);
		Octdata_EXPORTS ~Series();

		Octdata_EXPORTS const SloImage& getSloImage() const         { return *sloImage; }
		void takeSloImage(SloImage* sloImage);

		Octdata_EXPORTS const BScanList getBScans() const           { return bscans;    }
		Octdata_EXPORTS const BScan* getBScan(std::size_t pos) const;
		Octdata_EXPORTS std::size_t bscanCount() const              { return bscans.size(); }

		Octdata_EXPORTS Laterality getLaterality() const            { return laterality; }
		void setLaterality(Laterality l)                            { laterality = l; }

		Octdata_EXPORTS ScanPattern getScanPattern() const          { return scanPattern; }
		void setScanPattern(ScanPattern p)                          { scanPattern = p;    }
		
		void setScanDate(const Date& time)                          { scanDate = time; }
		Octdata_EXPORTS const Date& getScanDate() const             { return scanDate; }
		
		void setSeriesUID(const std::string& uid)                   { seriesUID = uid;  }
		Octdata_EXPORTS const std::string& getSeriesUID() const     { return seriesUID; }
		
		void setRefSeriesUID(const std::string& uid)                { refSeriesID = uid;  }
		Octdata_EXPORTS const std::string& getRefSeriesUID() const  { return refSeriesID; }

		void setScanFocus(double focus)                             { scanFocus = focus; }
		Octdata_EXPORTS   double getScanFocus()              const  { return scanFocus;  }

		void takeBScan(BScan* bscan);

		Octdata_EXPORTS int getInternalId() const                   { return internalId; }
	private:
		const int internalId;

		SloImage*                               sloImage = nullptr;
		std::string                             seriesUID;
		std::string                             refSeriesID;
		double                                  scanFocus;

		Laterality                              laterality = Laterality::undef;
		ScanPattern                             scanPattern = ScanPattern::Unknown;
		Date                                    scanDate;

		BScanList                               bscans;
	};

}
