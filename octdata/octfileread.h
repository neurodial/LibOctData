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

#include <vector>
#include <string>

#include "octextension.h"

#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif

namespace boost { namespace filesystem { class path; } }

namespace CppFW { class Callback; }

namespace OctData
{
	class OCT;
	class OctFileReader;
	class FileReadOptions;
	class FileWriteOptions;
	class OctExtensionsList;
	class FileReader;

	class OctFileRead
	{
		friend class OctFileReader;
	public:
		Octdata_EXPORTS static OctFileRead& getInstance()                        { static OctFileRead instance; return instance; }

		Octdata_EXPORTS static const OctExtensionsList& supportedExtensions();
		Octdata_EXPORTS static OCT openFile(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback = nullptr);
		Octdata_EXPORTS static OCT openFile(const boost::filesystem::path& filename, const FileReadOptions& op, CppFW::Callback* callback = nullptr);
		Octdata_EXPORTS static OCT openFile(const std::string& filename, CppFW::Callback* callback = nullptr);

		Octdata_EXPORTS static bool isLoadable(const std::string& filename);

		Octdata_EXPORTS static bool writeFile(const std::string& filename, const OCT& octdata);
		Octdata_EXPORTS static bool writeFile(const std::string& filename, const OCT& octdata, const FileWriteOptions& opt);
		Octdata_EXPORTS static bool writeFile(const boost::filesystem::path& filepath, const OCT& octdata, const FileWriteOptions& opt);

	private:
		OctFileRead();
		~OctFileRead();

		void registerFileRead(OctFileReader* reader);
		OCT openFilePrivat(const std::string& filename, const FileReadOptions& op, CppFW::Callback* callback);
		OCT openFilePrivat(const boost::filesystem::path& file, const FileReadOptions& op, CppFW::Callback* callback);

		bool writeFilePrivat(const boost::filesystem::path& filepath, const OCT& octdata, const FileWriteOptions& opt);

		bool openFileFromExt(OCT& oct, FileReader& filename, const FileReadOptions& op, CppFW::Callback* callback);
		bool tryOpenFile(OCT& oct, FileReader& filename, const FileReadOptions& op, CppFW::Callback* callback);

		OctExtensionsList extensions;

		std::vector<OctFileReader*> fileReaders;
	};
	
}

