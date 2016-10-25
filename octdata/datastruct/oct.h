#pragma once

#include "substructure_template.h"
#include "patient.h"

namespace OctData
{
	class OCT : public SubstructureTemplate<Patient>
	{
	public:
		      Patient& getPatient(int patientId)                  { return getAndInsert        (patientId) ; }
		const Patient& getPatient(int patientId) const            { return *(substructureMap.at(patientId)); }
		void clear()                                              { clearSubstructure(); }

		void findSeries(const OctData::Series* series, const OctData::Patient*& pat, const OctData::Study*& study) const;
	};

}
