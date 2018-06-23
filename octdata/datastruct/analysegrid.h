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
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("center"     , p.center     );
			getSet("diametersMM", p.diametersMM);
		}
	public:
		const CoordSLOmm& getCenter()                             const { return center; }
		const std::vector<double>& getDiametersMM()               const { return diametersMM; }

		void setCenter(const CoordSLOmm& c)                             { center = c; }
		void addDiameterMM(double diameter)                             { diametersMM.push_back(diameter); }
	};

}
