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
