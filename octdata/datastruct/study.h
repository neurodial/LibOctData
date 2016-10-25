#pragma once

#include "substructure_template.h"
#include "series.h"

namespace OctData
{
	class Study : public SubstructureTemplate<Series>
	{
		std::string studyUID     ;
		std::string studyOperator;
		Date        studyDate    ;

		
		const int internalId;
	public:
		explicit Study(int internalId) : internalId(internalId)   {}

		      Series& getSeries(int seriesId)                     { return getAndInsert        (seriesId) ; }
		const Series& getSeries(int seriesId) const               { return *(substructureMap.at(seriesId)); }

		const std::string& getStudyUID() const                    { return studyUID; }
		void setStudyUID(const std::string& uid)                  { studyUID = uid;  }
		
		
		const std::string& getStudyOperator() const               { return studyOperator; }
		const Date& getStudyDate() const                          { return studyDate    ; }
		
		void setStudyOperator(const std::string& s)               { studyOperator = s; }
		void setStudyDate    (const Date&        d)               { studyDate     = d; }

		int getInternalId() const                                      { return internalId; }
	};

}
