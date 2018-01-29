#pragma once

#include "substructure_template.h"
#include "patient.h"

namespace OctData
{
	class OCT : public SubstructureTemplate<Patient>
	{
	public:
		Octdata_EXPORTS       Patient& getPatient(int patientId)                  { return getAndInsert        (patientId) ; }
		Octdata_EXPORTS const Patient& getPatient(int patientId) const            { return *(substructureMap.at(patientId)); }
		Octdata_EXPORTS void clear()                                              { clearSubstructure(); }

		Octdata_EXPORTS void findSeries(const OctData::Series* series, const OctData::Patient*& pat, const OctData::Study*& study) const;


		template<typename T> void getSetParameter(T& /*getSet*/)       { }
		template<typename T> void getSetParameter(T& /*getSet*/) const { }
	};

}
