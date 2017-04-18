#pragma once

#include <cmath>
#include <ostream>

namespace OctData
{

	class ScaleFactor
	{
		double x;
		double y;

	public:
		ScaleFactor(double x, double y) : x(x), y(y)                {}
		explicit ScaleFactor(double factor) : x(factor), y(factor)  {}
		ScaleFactor() : x(1), y(1)                                  {}

		double getX() const                                         { return x; }
		double getY() const                                         { return y; }

		ScaleFactor  operator* (double factor) const                { return ScaleFactor(x*factor, y*factor); }
		ScaleFactor& operator*=(double factor)                      { x *= factor; y *= factor; return *this; }
	};

	class CoordSLOpx
	{
		double x;
		double y;

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

		double abs(const CoordSLOpx& o) const                       { return std::sqrt((x-o.x)*(x-o.x) + (y-o.y)*(y-o.y)); }

		void print(std::ostream& stream) const                      { stream << "(" << x << " | " << y << ")"; }
	};

	inline std::ostream& operator<<(std::ostream& stream, const CoordSLOpx& obj) { obj.print(stream); return stream; }


	class CoordSLOmm
	{
		double x;
		double y;

		bool set = false;

	public:
		CoordSLOmm(double x, double y) : x(x), y(y), set(true)      {}
		CoordSLOmm() : x(0), y(0)                                   {}

		double getX() const                                         { return x; }
		double getY() const                                         { return y; }

		CoordSLOpx operator*(const ScaleFactor& factor) const       { return CoordSLOpx(x/factor.getX(), y/factor.getY()); }

		operator bool() const                                       { return set; }
	};


	class CoordTransform
	{
		double a11 = 1.;
		double a12 = 0.;
		double a21 = 0.;
		double a22 = 1.;

		double b1 = 0.;
		double b2 = 0.;
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
	};

}
