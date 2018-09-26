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

#include "filereader.h"

#include<boost/endian/conversion.hpp>

#include"filestreamdircet.h"
#include"filestreamgzip.h"
#include <import/platform_helper.h>

namespace bfs = boost::filesystem;

namespace OctData
{

	FileReader::FileReader(const boost::filesystem::path& filepath)
	: filepath(filepath)
	{
		const boost::filesystem::path ext = filepath.extension();
		if(filepath.extension() == ".gz")
		{
			compressType = Compressed::gzip;
			extension = filepath.stem().extension();
		}
		else
		{
			compressType = Compressed::none;
			extension = ext;
		}
	}

	FileReader::~FileReader()
	{
		delete fileStream;
	}

	bool OctData::FileReader::openFile()
	{
		delete fileStream;
		fileStream = nullptr;
		switch(compressType)
		{
			case Compressed::none:
				fileStream = new FileStreamDircet(filepath);
				break;
			case Compressed::gzip:
#ifdef WITH_ZLIB
				fileStream = new FileStreamGZip(filepath);
#endif
				break;
		}
		return fileStream != nullptr;
	}

	std::size_t FileReader::file_size() const
	{
		if(filesize == 0)
		{
			switch(compressType)
			{
				case Compressed::none:
					filesize = bfs::file_size(filepath);
					break;
				case Compressed::gzip:
					std::ifstream stream(filepathConv(filepath), std::ios::binary | std::ios::in);
					stream.seekg(-4, std::ios_base::end);
					unsigned int size;
					stream.read(reinterpret_cast<char*>(&size), sizeof(size));
					boost::endian::little_to_native_inplace(size);
					filesize = size;
					break;
			}
		}

		return filesize;
	}




}
