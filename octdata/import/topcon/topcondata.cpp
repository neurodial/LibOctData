#include"topcondata.h"

#define _USE_MATH_DEFINES
#include<cmath>

namespace
{

	void applyParamScan(TopconData& data)
	{
		if(data.bscanList.size() == 0)
			return;

		const double resYmm = data.scanParameter.scanSizeYmm/static_cast<double>(data.bscanList.size());
		const double resZmm = data.scanParameter.resZum/1000;
		for(TopconData::BScanPair& bscan : data.bscanList)
		{
			double resXmm = data.scanParameter.scanSizeXmm/static_cast<double>(bscan.image.cols);
			switch(bscan.data.bscanType)
			{
				case OctData::BScan::BScanType::Circle: bscan.data.scaleFactor = OctData::ScaleFactor(resXmm*M_PI, resYmm, resZmm); break;
				case OctData::BScan::BScanType::Line  : bscan.data.scaleFactor = OctData::ScaleFactor(resXmm     , resYmm, resZmm); break;
				case OctData::BScan::BScanType::Unknown: break;
			}
		}
	}


	void applyRegistInfoVol(const TopconData::ScanParameter& parameter, TopconData::BScanPair& bscan, double pos)
	{
		bscan.data.start = OctData::CoordSLOmm(0                    , parameter.scanSizeYmm*pos);
		bscan.data.end   = OctData::CoordSLOmm(parameter.scanSizeXmm, parameter.scanSizeYmm*pos);
	}
	void applyRegistInfoCircle(const TopconData::ScanParameter& parameter, TopconData::BScanPair& bscan)
	{
		bscan.data.start  = OctData::CoordSLOmm(parameter.scanSizeXmm/2., 0);
		bscan.data.center = OctData::CoordSLOmm(0                       , 0);
	}

	void applyBscanCoords(TopconData& data)
	{
		TopconData::BScanList& list = data.bscanList;

		const std::size_t numBScans = list.size();

		std::size_t bscanNum = 0;
		for(TopconData::BScanPair& bscan : list)
		{
			double pos = static_cast<double>(bscanNum)/static_cast<double>(numBScans);
			switch(bscan.data.bscanType)
			{
				case OctData::BScan::BScanType::Circle : applyRegistInfoCircle(data.scanParameter, bscan); break;
				case OctData::BScan::BScanType::Line   : applyRegistInfoVol   (data.scanParameter, bscan, pos); break;
				case OctData::BScan::BScanType::Unknown: break;
			}

			++bscanNum;
		}
	}

	void applySloDataRect(TopconData& data, TopconData::SloData& sloData)
	{
		const double scanSloSizeXpx = sloData.registData.maxX - sloData.registData.minX;
		const double scanSloSizeYpx = sloData.registData.maxY - sloData.registData.minY;

		const TopconData::ScanParameter& parameter = data.scanParameter;

		double sloScaleX = parameter.scanSizeXmm/scanSloSizeXpx;
		double sloScaleY = parameter.scanSizeYmm/scanSloSizeYpx;

		sloData.sloImage->setScaleFactor(OctData::ScaleFactor(sloScaleX, sloScaleY));
		sloData.sloImage->setShift(OctData::CoordSLOpx(sloData.registData.minX, sloData.registData.minY));

	}
	void applySloDataCircle(TopconData& data, TopconData::SloData& sloData)
	{
		const double scanSloSizePx = sloData.registData.radius;

		const TopconData::ScanParameter& parameter = data.scanParameter;

		double sloScaleX = parameter.scanSizeXmm/scanSloSizePx;
		double sloScaleY = parameter.scanSizeXmm/scanSloSizePx;

		sloData.sloImage->setScaleFactor(OctData::ScaleFactor(sloScaleX, sloScaleY));
		sloData.sloImage->setShift(OctData::CoordSLOpx(sloData.registData.centerX, sloData.registData.centerY));
	}

	void applySloData(TopconData& data, TopconData::SloData& sloData)
	{
		switch(data.series.getScanPattern())
		{
			case OctData::Series::ScanPattern::Circular: applySloDataCircle(data, sloData); break;
			case OctData::Series::ScanPattern::Volume  : applySloDataRect  (data, sloData); break;
			default: break;
		}
		data.series.takeSloImage(sloData.sloImage);
	}
}


TopconData::TopconData(OctData::OCT& oct)
: oct   (oct)
, pat   (oct.getPatient (1))
, study (pat.getStudy   (1))
, series(study.getSeries(1))
{
}



void TopconData::transferData2Series()
{
	     if(sloFundus.sloImage) applySloData(*this, sloFundus);
	else if(sloTRC   .sloImage) applySloData(*this, sloTRC   );

	applyParamScan(*this);
	applyBscanCoords(*this);

	for(BScanPair& pair : bscanList)
	{
		OctData::BScan::Data bscanData;
		OctData::BScan* bscan = new OctData::BScan(pair.image, pair.data);

		series.takeBScan(bscan);
	}
}
