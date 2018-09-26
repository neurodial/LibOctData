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

#include<string>
#include"datastruct/objectwrapper.h"

namespace OctData
{
	class FileReadOptions
	{
	public:
		enum class E2eGrayTransform { nativ, xml, vol, u16 };
		typedef ObjectWrapper<E2eGrayTransform> E2eGrayTransformEnumWrapper;

		bool fillEmptyPixelWhite = true;
		bool registerBScanns     = true;
		bool rotateSlo           = false;

		bool holdRawData         = false;
		bool loadRefFiles        = true;
		bool readBScans          = true;

		bool dumpFileParts       = false;

		E2eGrayTransform e2eGray = E2eGrayTransform::xml;

		std::string libPath;

		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }

	private:
		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			E2eGrayTransformEnumWrapper e2eGrayWrapper(p.e2eGray);

			getSet("fillEmptyPixelWhite", p.fillEmptyPixelWhite                    );
			getSet("registerBScanns"    , p.registerBScanns                        );
			getSet("rotateSlo"          , p.rotateSlo                              );
			getSet("holdRawData"        , p.holdRawData                            );
			getSet("loadRefFiles"       , p.loadRefFiles                           );
			getSet("readBScans"         , p.readBScans                             );
			getSet("e2eGrayTransform"   , static_cast<std::string&>(e2eGrayWrapper));
		}
	};
}
