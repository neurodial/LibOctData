#pragma once

#include "../octfilereader.h"

namespace OctData
{
	class FileReadOptions;

	/**
	 * hidef scan consists of 2 orthogonal bscans intersecting in the center of
	 * the volume. The first is in the y direction (across B-scans), the second
	 * is in the x direction (aligned with the center B-scan). Unfortunately,
	 * they are not in good alignment with the volumetric data
	 */
	class CirrusRawRead : public OctFileReader
	{
	public:
		CirrusRawRead();

		virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};

}

