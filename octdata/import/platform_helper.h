#pragma once

#include <string>

#include <boost/predef.h>
#include <boost/filesystem.hpp>



#if BOOST_OS_WINDOWS
std::wstring toUtf16(std::string str)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	if (len > 0)
	{
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
	}
return ret;
}

std::wstring filenameConv(const boost::filesystem::path& file)
{
	return file.generic_string();
}
#else
const std::string& filenameConv(const boost::filesystem::path& file)
{
	return file.generic_string();
}

#endif

