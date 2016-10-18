#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "date.h"

namespace OctData
{
	class SloImage;
	class BScan;

	class Series
	{
		Series(const Series&)            = delete;
		Series& operator=(const Series&) = delete;

	public:
		enum class Laterality { undef, OD, OS};

		typedef std::vector<BScan*> BScanList;
		Series();
		~Series();

		const SloImage& getSloImage() const                         { return *sloImage; }
		void takeSloImage(SloImage* sloImage);
		const BScanList getBScans() const                           { return bscans;    }
		const BScan* getBScan(std::size_t pos) const;
		std::size_t bscanCount() const                              { return bscans.size(); }

		void setLaterality(Laterality l)                            { laterality = l; }
		Laterality getLaterality() const                            { return laterality; }
		
		const Date& getTime() const                                 { return scanDate; }
		void setTime(const Date& time)                              { scanDate = time; }
		
		const std::string& getSeriesUID() const                     { return seriesUID; }
		void setSeriesUID(const std::string& uid)                   { seriesUID = uid;  }
		
		const std::string& getRefSeriesUID() const                  { return refSeriesID; }
		void setRefSeriesUID(const std::string& uid)                { refSeriesID = uid;  }

		void takeBScan(BScan* bscan);
	private:
		SloImage*                               sloImage = nullptr;
		std::string                             seriesUID;
		std::string                             refSeriesID;

		Laterality                              laterality = Laterality::undef;
		Date                                    scanDate;

		BScanList                               bscans;
	};

}
