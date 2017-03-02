#pragma once

namespace OctData
{
	class FileReadOptions
	{
	public:
		enum class E2eGrayTransform { nativ, xml, vol };

		bool fillEmptyPixelWhite = true;
		bool registerBScanns     = true;
		bool rotateSlo           = false;

		bool holdRawData         = false;

		bool loadRefFiles        = true;

		E2eGrayTransform e2eGray = E2eGrayTransform::xml;
	};
}
