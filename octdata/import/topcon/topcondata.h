#pragma once

#include<opencv/cv.hpp>
#include<datastruct/bscan.h>
#include<datastruct/series.h>
#include<datastruct/sloimage.h>

#include<datastruct/oct.h>



struct TopconData
{
	TopconData(OctData::OCT& oct);
	~TopconData() { transferData2Series(); }

	void transferData2Series();

	struct BScanPair
	{
		cv::Mat image;
		OctData::BScan::Data data;
	};

	struct ScanParameter
	{
		double scanSizeXmm = 0;
		double scanSizeYmm = 0;
		double resZum      = 0;
	};

	typedef std::vector<BScanPair> BScanList;


	class SloRegistData
	{
	public:
		double maxX = 0;
		double maxY = 0;
		double minX = 0;
		double minY = 0;


		SloRegistData() = default;

		SloRegistData(uint32_t v[4])
		: maxX(v[2])
		, maxY(v[3])
		, minX(v[0])
		, minY(v[1])
		{}
	};

	struct SloData
	{
		OctData::SloImage* sloImage = nullptr;
		SloRegistData registData;
	};


	BScanList bscanList;
	SloData   sloData;
	ScanParameter scanParameter;

	OctData::OCT&     oct   ;
	OctData::Patient& pat   ;
	OctData::Study&   study ;
	OctData::Series&  series;

	SloData sloFundus;
	SloData sloTRC   ;
};

