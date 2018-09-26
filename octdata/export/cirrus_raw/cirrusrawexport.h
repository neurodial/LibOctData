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

namespace boost { namespace filesystem { class path; } }


namespace OctData
{
	class OCT;
	class Study;
	class Series;
	class Patient;
	class FileWriteOptions;

	class CirrusRawExport
	{
	public:
		static bool writeFile(const boost::filesystem::path& file, const OCT& oct, const FileWriteOptions& opt);
		static bool writeFile(const boost::filesystem::path& file, const OCT& oct, const Patient& pat, const Study& study, const Series& series, const FileWriteOptions& opt);
	};
}
