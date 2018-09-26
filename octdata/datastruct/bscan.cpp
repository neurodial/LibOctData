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

#include "bscan.h"

#define _USE_MATH_DEFINES
#include<cmath>
#include<exception>

#include <opencv/cv.hpp>


namespace OctData
{

	BScan::BScan(const cv::Mat& img, const BScan::Data& data)
	: image     (new cv::Mat(img))
	, angioImage(new cv::Mat)
	, rawImage  (new cv::Mat)
	, data      (data)
	{

	}

	BScan::~BScan()
	{
		delete image;
		delete angioImage;
		delete rawImage;
	}

	int BScan::getWidth() const
	{
		return image->cols;
	}

	int BScan::getHeight() const
	{
		return image->rows;
	}

	void BScan::setRawImage(const cv::Mat& img)
	{
		*rawImage = img;
	}

	void BScan::setAngioImage(const cv::Mat& img)
	{
		*angioImage = img;
	}

	OctData::CoordSLOmm calcCirclePos(const OctData::CoordSLOmm& center, const OctData::CoordSLOmm& start, double frac, bool clockwise)
	{
		if(!clockwise)
			frac = 1-frac;

		const double radius = center.abs(start);
		const double ratio  = start.getX() - center.getX();
		const double nullAngle = acos( ratio/radius )/M_PI/2;

		frac += nullAngle;
		frac *= 2*M_PI;

		const double posX = cos(frac)*radius + center.getX();
		const double posY = sin(frac)*radius + center.getY();

		return OctData::CoordSLOmm(posX, posY);
	}



	const OctData::CoordSLOmm OctData::BScan::getFracPos(double frac) const
	{
		switch(getBScanType())
		{
			case BScanType::Circle:
				return calcCirclePos(getCenter(), getStart(), frac, getClockwiseRot());
				break;
			case BScanType::Line:
				return getStart()*(1-frac) + getEnd()*(frac); // TODO: falsche Richtung?
				break;
			case BScanType::Unknown:
				break;
		}
		return OctData::CoordSLOmm();
	}

	const OctData::CoordSLOmm OctData::BScan::getAscanPos(std::size_t ascan) const
	{
		const std::size_t bscanWidth = getWidth();

		if(ascan >= bscanWidth && bscanWidth > 1)
			throw std::out_of_range("ascan number greater than bscan width");

		const double frac = static_cast<double>(ascan)/static_cast<double>(bscanWidth-1);
		return getFracPos(frac);
	}

	template<> void BScan::BScanTypeEnumWrapper::toString()
	{
		switch(obj)
		{
			case BScan::BScanType::Circle: std::string::operator=("Circle"); break;
			case BScan::BScanType::Line  : std::string::operator=("Line"  ); break;
			case BScan::BScanType::Unknown:
				break;
		}
	}
	template<> void BScan::BScanTypeEnumWrapper::fromString()
	{
		     if(*this == "Circle") obj = BScan::BScanType::Circle;
		else if(*this == "Line"  ) obj = BScan::BScanType::Line  ;
		else obj = BScan::BScanType::Unknown;
	}

}
