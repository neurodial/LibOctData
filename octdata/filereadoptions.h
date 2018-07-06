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
