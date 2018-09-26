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

#include <map>


#ifdef OCTDATA_EXPORT
	#include "octdata_EXPORTS.h"
#else
	#define Octdata_EXPORTS
#endif

namespace OctData
{

	template<typename Type, typename IndexType = int>
	class SubstructureTemplate
	{
		SubstructureTemplate(const SubstructureTemplate&)            = delete;
		SubstructureTemplate& operator=(const SubstructureTemplate&) = delete;
	public:
		Octdata_EXPORTS SubstructureTemplate()                                       = default;
		Octdata_EXPORTS SubstructureTemplate(SubstructureTemplate&& o)          { swapSubstructure(o); }

		SubstructureTemplate& operator=(SubstructureTemplate&& o)               { swapSubstructure(o); return *this; }

		typedef Type                                     SubstructureType;
		typedef std::pair<const IndexType, Type*>        SubstructurePair;
		typedef std::map<IndexType, Type*>               SubstructureMap;
		typedef typename SubstructureMap::iterator       SubstructureIterator;
		typedef typename SubstructureMap::const_iterator SubstructureCIterator;

		Octdata_EXPORTS std::size_t subStructureElements() const                { return substructureMap.size();  }

		Octdata_EXPORTS SubstructureCIterator begin() const                     { return substructureMap.begin(); }
		Octdata_EXPORTS SubstructureCIterator end()   const                     { return substructureMap.end();   }
		Octdata_EXPORTS SubstructureIterator  begin()                           { return substructureMap.begin(); }
		Octdata_EXPORTS SubstructureIterator  end()                             { return substructureMap.end();   }
		Octdata_EXPORTS std::size_t size()            const                     { return substructureMap.size();  }

	protected:
		void swapSubstructure(SubstructureTemplate& d)          { substructureMap.swap(d.substructureMap); }

		virtual ~SubstructureTemplate()
		{
			for(SubstructurePair obj : substructureMap)
				delete obj.second;
		}

		Type& getAndInsert(IndexType id)
		{
			SubstructureIterator it = substructureMap.find(id);
			if(it == substructureMap.end())
			{
				std::pair<SubstructureIterator, bool> pit = substructureMap.emplace(id, new Type(id));
				if(pit.second == false)
					throw "SubstructureTemplate pit.second == false";
				return *((pit.first)->second);
			}
			return *(it->second);
		};

		void clearSubstructure()
		{
			for(SubstructurePair obj : substructureMap)
				delete obj.second;
			substructureMap.clear();
		}


		SubstructureMap substructureMap;
	};


}
