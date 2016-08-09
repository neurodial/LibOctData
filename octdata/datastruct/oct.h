#pragma once

#include "substructure_template.h"
#include "patient.h"

namespace OctData
{
	class OCT : public SubstructureTemplate<Patient>
	{
	public:
		      Patient& getPatient(int patientId)                     { return getAndInsert        (patientId) ; }
		const Patient& getPatient(int patientId) const               { return *(substructureMap.at(patientId)); }
	};

}
