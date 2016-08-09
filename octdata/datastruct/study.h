#pragma once

#include "substructure_template.h"
#include "series.h"

namespace OctData
{
	class Study : public SubstructureTemplate<Series>
	{
	public:
		      Series& getSeries(int seriesId)                     { return getAndInsert        (seriesId) ; }
		const Series& getSeries(int seriesId) const               { return *(substructureMap.at(seriesId)); }
	};

}
