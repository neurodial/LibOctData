#include "series.h"

#include "bscan.h"
#include "sloimage.h"

#include <limits>

namespace OctData
{

	Series::Series(int internalId)
	: internalId(internalId)
	, sloImage(new SloImage)
	, scanFocus(std::numeric_limits<double>::quiet_NaN())
	{

	}


	Series::~Series()
	{
		for(BScan* bscan : bscans)
			delete bscan;

		delete sloImage;
	}

	void Series::takeBScan(OctData::BScan* bscan)
	{
		bscans.push_back(bscan);

	}

	const BScan* Series::getBScan(std::size_t pos) const
	{
		if(pos >= bscans.size())
			return nullptr;
		return bscans[pos];
	}

	void Series::takeSloImage(SloImage* slo)
	{
		if(slo)
		{
			delete sloImage;
			sloImage = slo;
		}
	}


}
