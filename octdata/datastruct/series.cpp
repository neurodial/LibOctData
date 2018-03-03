#include "series.h"

#include "bscan.h"
#include "sloimage.h"

#include <limits>

#define _USE_MATH_DEFINES
#include <cmath>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/algorithms/assign.hpp>

namespace OctData
{

	Series::Series(int internalId)
	: internalId(internalId)
	, sloImage(new SloImage)
	, scanFocus(std::numeric_limits<double>::quiet_NaN())
	{

	}


	Series::~Series()
	{
		for(BScan* bscan : bscans)
			delete bscan;

		delete sloImage;
	}

	void Series::takeBScan(OctData::BScan* bscan)
	{
		bscans.push_back(bscan);
		calculateSLOConvexHull();
		updateCornerCoords();
	}

	const BScan* Series::getBScan(std::size_t pos) const
	{
		if(pos >= bscans.size())
			return nullptr;
		return bscans[pos];
	}

	void Series::takeSloImage(SloImage* slo)
	{
		if(slo)
		{
			delete sloImage;
			sloImage = slo;
		}
	}



	void Series::updateCornerCoords(const CoordSLOmm& point)
	{
		double minX = std::min(point.getX(), leftUpper .getX());
		double maxX = std::max(point.getX(), rightLower.getX());
		double minY = std::min(point.getY(), leftUpper .getY());
		double maxY = std::max(point.getY(), rightLower.getY());

		leftUpper  = CoordSLOmm(minX, minY);
		rightLower = CoordSLOmm(maxX, maxY);
	}

	void Series::updateCornerCoords()
	{
		BScan* bscan = bscans.back();
		if(!bscan)
			return;

		if(bscans.size() == 1) // first scan, init points
		{
			leftUpper  = bscan->getStart();
			rightLower = bscan->getStart();
		}

		switch(bscan->getBScanType())
		{
			case BScan::BScanType::Circle:
			{
				double radius = bscan->getCenter().abs(bscan->getStart());
				updateCornerCoords(bscan->getCenter() + CoordSLOmm( radius, 0));
				updateCornerCoords(bscan->getCenter() + CoordSLOmm(-radius, 0));
				updateCornerCoords(bscan->getCenter() + CoordSLOmm(0,  radius));
				updateCornerCoords(bscan->getCenter() + CoordSLOmm(0, -radius));
				break;
			}
			case BScan::BScanType::Line:
			case BScan::BScanType::Unknown:
				updateCornerCoords(bscan->getStart());
				updateCornerCoords(bscan->getEnd());
				break;
		}
	}

	void Series::calculateSLOConvexHull()
	{
		convexHullSLOBScans.clear();

		typedef boost::geometry::model::d2::point_xy<double> Point;
		typedef boost::geometry::model::polygon<Point> Polygon;
		typedef std::vector<Point> PointsList;

		struct Adder
		{
			static void addPoint(PointsList& pl, const CoordSLOmm& pt)
			{
				pl.emplace_back(pt.getX(), pt.getY());
			}
			static void addCircle(PointsList& pl, const CoordSLOmm& center, const CoordSLOmm& pt)
			{
				double radius = center.abs(pt);
				for(double alpha = 0; alpha < 2*M_PI; alpha += M_PI/16)
				{
					CoordSLOmm p = center + CoordSLOmm(sin(alpha)*radius, cos(alpha)*radius);
					pl.emplace_back(p.getX(), p.getY());
				}
			}
		};

		PointsList points;
		for(BScan* bscan : bscans)
		{
			if(bscan)
			{
				if(bscan->getCenter())
					Adder::addCircle(points, bscan->getCenter(), bscan->getStart());
				else
				{
					Adder::addPoint(points, bscan->getStart());
					Adder::addPoint(points, bscan->getEnd());
				}
			}
		}

		Polygon poly;
		boost::geometry::assign_points(poly, points);

		Polygon hull;
		boost::geometry::convex_hull(poly, hull);

		// ring is a vector
		std::vector<Point> const& convexPoints = hull.outer();
		for(const Point&p : convexPoints)
			convexHullSLOBScans.emplace_back(p.get<0>(), p.get<1>());
	}


	template<> void Series::LateralityEnumWrapper::toString()
	{
		switch(obj)
		{
			case Series::Laterality::OD: std::string::operator=("OD"); break;
			case Series::Laterality::OS: std::string::operator=("OS"); break;
			case Series::Laterality::undef:
				break;
		}
	}
	template<> void Series::LateralityEnumWrapper::fromString()
	{
		     if(*this == "OD") obj = Series::Laterality::OD;
		else if(*this == "OS") obj = Series::Laterality::OS;
		else obj = Series::Laterality::undef;
	}


	template<> void Series::ScanPatternEnumWrapper::toString()
	{
		switch(obj)
		{
			case Series::ScanPattern::Text         : std::string::operator=("Text"         ); break;
			case Series::ScanPattern::SingleLine   : std::string::operator=("SingleLine"   ); break;
			case Series::ScanPattern::Circular     : std::string::operator=("Circular"     ); break;
			case Series::ScanPattern::Volume       : std::string::operator=("Volume"       ); break;
			case Series::ScanPattern::FastVolume   : std::string::operator=("FastVolume"   ); break;
			case Series::ScanPattern::Radial       : std::string::operator=("Radial"       ); break;
			case Series::ScanPattern::RadialCircles: std::string::operator=("RadialCircles"); break;
			case Series::ScanPattern::Unknown:
				break;
		}
	}
	template<> void Series::ScanPatternEnumWrapper::fromString()
	{
		     if(*this == "Text"         ) obj = Series::ScanPattern::Text          ;
		else if(*this == "SingleLine"   ) obj = Series::ScanPattern::SingleLine    ;
		else if(*this == "Circular"     ) obj = Series::ScanPattern::Circular      ;
		else if(*this == "Volume"       ) obj = Series::ScanPattern::Volume        ;
		else if(*this == "FastVolume"   ) obj = Series::ScanPattern::FastVolume    ;
		else if(*this == "Radial"       ) obj = Series::ScanPattern::Radial        ;
		else if(*this == "RadialCircles") obj = Series::ScanPattern::RadialCircles ;
		else obj = Series::ScanPattern::Unknown;
	}



	template<> void Series::ExaminedStructureEnumWrapper::toString()
	{
		switch(obj)
		{
			case Series::ExaminedStructure::Text  : std::string::operator=("Text"  ); break;
			case Series::ExaminedStructure::ONH   : std::string::operator=("ONH"   ); break;
			case Series::ExaminedStructure::Retina: std::string::operator=("Retina"); break;
			case Series::ExaminedStructure::Unknown:
				break;
		}
	}
	template<> void Series::ExaminedStructureEnumWrapper::fromString()
	{
		     if(*this == "Text"  ) obj = Series::ExaminedStructure::Text  ;
		else if(*this == "ONH"   ) obj = Series::ExaminedStructure::ONH   ;
		else if(*this == "Retina") obj = Series::ExaminedStructure::Retina;
		else obj = Series::ExaminedStructure::Unknown;
	}


}
