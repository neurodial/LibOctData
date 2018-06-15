#pragma once

#include<string>

namespace OctData
{
	class FileReadOptions
	{
	public:
		enum class E2eGrayTransform { nativ, xml, vol, u16 };

		bool fillEmptyPixelWhite = true;
		bool registerBScanns     = true;
		bool rotateSlo           = false;

		bool holdRawData         = false;

		bool loadRefFiles        = true;

		bool readBScans          = true;

		E2eGrayTransform e2eGray = E2eGrayTransform::xml;

		std::string libPath;
	};
}
