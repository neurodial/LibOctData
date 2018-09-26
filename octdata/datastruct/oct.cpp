/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
