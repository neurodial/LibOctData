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

#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif


#include<string>

namespace OctData
{
	template<typename T>
	class ObjectWrapper : public std::string
	{
		T constObj; // temporary for const usage
		T& obj;

		Octdata_EXPORTS void toString();
		Octdata_EXPORTS void fromString();

	public:
		ObjectWrapper(const T& o)
		: constObj(o)
		, obj(constObj)
		{ toString(); }

		ObjectWrapper(T& o)
		: obj(o)
		{ toString(); }

		~ObjectWrapper()
		{
			if(&constObj != &obj)
				fromString();
		}
	};
}
