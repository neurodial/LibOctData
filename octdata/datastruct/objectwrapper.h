#pragma once


#include<string>

namespace OctData
{
	template<typename T>
	class ObjectWrapper : public std::string
	{
		T constObj; // temporary for const usage
		T& obj;

		void toString();
		void fromString();

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
