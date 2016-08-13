#pragma once

#include <string>
#include <vector>

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

		void takeBScan(BScan* bscan);
	private:
		SloImage*                               sloImage = nullptr;
		std::string                             seriesID;
		std::string                             refSeriesID;

		Laterality                              laterality = Laterality::undef;

		BScanList                               bscans;
	};

}
