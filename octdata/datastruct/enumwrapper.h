#pragma once


#include<string>

namespace OctData
{
	template<typename T>
	class EnumWrapper : public std::string
	{
		T constObj; // temporary for const usage
		T& obj;

		void toString();
		void fromString();

	public:
		EnumWrapper(const T& o)
		: constObj(o)
		, obj(constObj)
		{ toString(); }

		EnumWrapper(T& o)
		: obj(o)
		{ toString(); }

		~EnumWrapper()
		{
			if(&constObj != &obj)
				fromString();
		}
	};
}
