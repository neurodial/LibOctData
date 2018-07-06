#pragma once

#include<string>
#include"datastruct/objectwrapper.h"


namespace OctData
{
	class FileWriteOptions
	{
	public:
		enum class XoctImageFormat { png, tiff, bmp, zippedBMP };
		typedef ObjectWrapper<XoctImageFormat> XoctImageFormatEnumWrapper;


		bool            octBinFlat      = false;
		XoctImageFormat xoctImageFormat = XoctImageFormat::png;


		template<typename T> void getSetParameter(T& getSet)           { getSetParameter(getSet, *this); }
		template<typename T> void getSetParameter(T& getSet)     const { getSetParameter(getSet, *this); }

	private:
		template<typename T, typename ParameterSet>
		static void getSetParameter(T& getSet, ParameterSet& p)
		{
			XoctImageFormatEnumWrapper xoctImageFormat(p.xoctImageFormat);

			getSet("octBinFlat"     , p.octBinFlat                              );
			getSet("xoctImageFormat", static_cast<std::string&>(xoctImageFormat));
		}
	};
}

