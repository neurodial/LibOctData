#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace OctData
{
	class SloImage;
	class BScan;

	class Series
	{
		Series(const Series&)            = delete;
		Series& operator=(const Series&) = delete;

	public:
		enum class Laterality { undef, OD, LD};

		typedef std::chrono::system_clock::time_point Time;
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
		
		const Time& getTime() const                                 { return examTime; }
		void setTime(const Time& time)                              { examTime = time; }

		void takeBScan(BScan* bscan);
	private:
		SloImage*                               sloImage = nullptr;
		std::string                             seriesID;
		std::string                             refSeriesID;

		Laterality                              laterality = Laterality::undef;

		BScanList                               bscans;
		
		Time                                    examTime;
	};

}
