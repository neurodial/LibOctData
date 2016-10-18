#pragma once

#include "substructure_template.h"
#include "series.h"

namespace OctData
{
	class Study : public SubstructureTemplate<Series>
	{
		std::string studyUID;
	public:
		      Series& getSeries(int seriesId)                     { return getAndInsert        (seriesId) ; }
		const Series& getSeries(int seriesId) const               { return *(substructureMap.at(seriesId)); }

		const std::string& getStudyUID() const                    { return studyUID; }
		void setStudyUID(const std::string& uid)                  { studyUID = uid;  }
	};

}
