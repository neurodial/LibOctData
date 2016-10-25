#include "oct.h"

namespace OctData
{
	void OCT::findSeries(const OctData::Series* const seriesReq, const OctData::Patient*& pat, const OctData::Study*& study) const
	{
		pat   = nullptr;
		study = nullptr;

		if(seriesReq == nullptr)
			return;

		// Search series
		for(const OCT::SubstructurePair& patientPair : *this)
		{
			const Patient* actPatient = patientPair.second;
			for(const Patient::SubstructurePair& studyPair : *actPatient)
			{
				const Study* actStudy = studyPair.second;
				for(const Study::SubstructurePair& seriesPair : *actStudy)
				{
					const Series* series = seriesPair.second;
					if(series == seriesReq)
					{
						pat   = actPatient;
						study = actStudy;
						return;
					}
				}
			}
		}
	}
}
