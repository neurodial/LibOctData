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

#pragma once

#include<vector>

#include"coordslo.h"

namespace OctData
{
	class AnalyseGrid
	{
		CoordSLOmm center;
		std::vector<double> diametersMM;


		template<typename T, typename ParameterSet>
		static void callSubset(T& getSet, ParameterSet& p, const std::string& name)
		{
			T subSetGetSet = getSet.subSet(name);
			p.getSetParameter(subSetGetSet);
		}

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			callSubset(getSet, p.center, "center");
			getSet("diametersMM", p.diametersMM);
		}
	public:
		const CoordSLOmm& getCenter()                             const { return center; }
		const std::vector<double>& getDiametersMM()               const { return diametersMM; }

		void setCenter(const CoordSLOmm& c)                             { center = c; }
		void addDiameterMM(double diameter)                             { diametersMM.push_back(diameter); }


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }
	};

}
