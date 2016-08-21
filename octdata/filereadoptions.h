#pragma once

namespace OctData
{
	struct FileReadOptions
	{
		bool fillEmptyPixelWhite = true;
		bool registerBScanns     = true;
	};
}