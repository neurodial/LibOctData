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
