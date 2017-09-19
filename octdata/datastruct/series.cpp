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


	void OctData::Series::calculateSLOConvexHull()
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

}
