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

#include <cmath>
#include <ostream>

namespace OctData
{
	class CoordSLOmm;

	class ScaleFactor
	{
		double x;
		double y;
		double z = 0;

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("x", p.x);
			getSet("y", p.y);
			getSet("z", p.z);
		}

	public:
		ScaleFactor(double x, double y, double z = 0) : x(x), y(y), z(z)
		                                                               {}
		explicit ScaleFactor(double factor) : x(factor), y(factor)     {}
		ScaleFactor() : x(1), y(1)                                     {}

		double getX() const                                            { return x; }
		double getY() const                                            { return y; }
		double getZ() const                                            { return z; }

		ScaleFactor  operator* (double factor) const                   { return ScaleFactor(x*factor, y*factor, z*factor); }
		ScaleFactor& operator*=(double factor)                         { x *= factor; y *= factor; z*= factor; return *this; }

		void print(std::ostream& stream) const                         { stream << "(" << x << " | " << y << " | " << z << ")"; }

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }
	};
	inline std::ostream& operator<<(std::ostream& stream, const ScaleFactor& obj) { obj.print(stream); return stream; }

	class CoordSLOpx
	{
		double x;
		double y;

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("x", p.x);
			getSet("y", p.y);
		}

	public:
		CoordSLOpx(double x, double y) : x(x), y(y)                 {}
		CoordSLOpx() : x(0), y(0)                                   {}

		int getX() const                                            { return static_cast<int>(std::round(x)); }
		int getY() const                                            { return static_cast<int>(std::round(y)); }


		double getXf() const                                        { return x; }
		double getYf() const                                        { return y; }

		CoordSLOpx operator+(const CoordSLOpx& o) const             { return CoordSLOpx(x+o.x   , y+o.y   ); }
		CoordSLOpx operator*(double factor)       const             { return CoordSLOpx(x*factor, y*factor); }
		CoordSLOpx operator-(const CoordSLOpx& v) const             { return CoordSLOpx(x-v.x   , y-v.y   ); }

		double operator*(const CoordSLOpx& v) const                 { return x*v.x + y*v.y; }
		double normquadrat()                  const                 { return x*x + y*y; }

		double absQuad(const CoordSLOpx& o) const                   { return (x-o.x)*(x-o.x) + (y-o.y)*(y-o.y); }
		double abs(const CoordSLOpx& o) const                       { return std::sqrt(absQuad(o)); }

		void print(std::ostream& stream) const                      { stream << "(" << x << " | " << y << ")"; }

		CoordSLOmm operator/(const ScaleFactor& factor) const;

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }
	};

	inline std::ostream& operator<<(std::ostream& stream, const CoordSLOpx& obj) { obj.print(stream); return stream; }


	class CoordSLOmm
	{
		double x;
		double y;

		bool set = false;

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("x"  , p.x  );
			getSet("y"  , p.y  );
			getSet("set", p.set);
		}

	public:
		CoordSLOmm(double x, double y) : x(x), y(y), set(true)      {}
		CoordSLOmm() : x(0), y(0)                                   {}

		double getX() const                                         { return x; }
		double getY() const                                         { return y; }

		CoordSLOpx operator*(const ScaleFactor& factor) const       { return CoordSLOpx(x/factor.getX(), y/factor.getY()); }

		CoordSLOmm operator+(const CoordSLOmm& o) const             { return CoordSLOmm(x+o.x   , y+o.y   ); }
		CoordSLOmm operator*(double factor)       const             { return CoordSLOmm(x*factor, y*factor); }
		CoordSLOmm operator-(const CoordSLOmm& v) const             { return CoordSLOmm(x-v.x   , y-v.y   ); }

		double abs(const CoordSLOmm& o) const                       { return std::sqrt((x-o.x)*(x-o.x) + (y-o.y)*(y-o.y)); }

		operator bool() const                                       { return set; }

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }
	};


	class CoordTransform
	{
		double a11 = 1.;
		double a12 = 0.;
		double a21 = 0.;
		double a22 = 1.;

		double b1 = 0.;
		double b2 = 0.;

		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			getSet("a11", p.a11);
			getSet("a12", p.a12);
			getSet("a21", p.a21);
			getSet("a22", p.a22);

			getSet("b1" , p.b1 );
			getSet("b2" , p.b2 );
		}
	public:
		CoordTransform() = default;
		CoordTransform(double a11, double a12, double a21, double a22, double b1, double b2)
		: a11(a11)
		, a12(a12)
		, a21(a21)
		, a22(a22)
		, b1 (b1 )
		, b2 (b2 )
		{}

		CoordSLOmm operator*(const CoordSLOmm& mm) const            { return CoordSLOmm(a11*mm.getX()  + a12*mm.getY()  + b1, a21*mm.getX()  + a22*mm.getY()  + b2); }
		CoordSLOpx operator*(const CoordSLOpx& px) const            { return CoordSLOpx(a11*px.getXf() + a12*px.getYf() + b1, a21*px.getXf() + a22*px.getYf() + b2); }

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }

		CoordTransform inv() const
		{
			double det = a11*a22 - a12*a21;
			if(det < 1e-7)
				return CoordTransform();
			double invDet = 1./det;
			return CoordTransform( a22*invDet
			                    , -a12*invDet
			                    , -a21*invDet
			                    ,  a11*invDet
			                    , -b1
			                    , -b2);
		}
	};



	inline CoordSLOmm CoordSLOpx::operator/(const ScaleFactor& factor) const { return CoordSLOmm(x*factor.getX(), y*factor.getY()); }
}
