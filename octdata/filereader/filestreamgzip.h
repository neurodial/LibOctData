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

#pragma once

#ifdef WITH_ZLIB

#include "filereader.h"
#include <zlib.h>


namespace OctData
{

	class FileStreamGZip : public FileStreamInterface
	{
		gzFile file;
	public:
		FileStreamGZip(const boost::filesystem::path& filepath);
		virtual ~FileStreamGZip();


		virtual std::streamsize read(char* dest, std::streamsize size) override
		                                                               { return gzread(file, dest, static_cast<unsigned>(size)); }
		virtual void seekg(std::streamoff pos)                override { gzseek(file, pos, SEEK_SET); }
		virtual bool good()                             const override { return !gzeof(file); }
	};

}

#endif
