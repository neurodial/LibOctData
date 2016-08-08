#include "series.h"

#include "bscan.h"
#include "sloimage.h"

namespace OctData
{

	Series::Series()
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

}
