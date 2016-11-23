#pragma once

#include <string>

#include <boost/predef.h>
#include <boost/filesystem.hpp>



#if BOOST_OS_WINDOWS
	#include <codecvt>

	inline std::wstring filenameConv(const std::string& utf_str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(utf_str);
	}

	inline std::wstring filepathConv(const boost::filesystem::path& file)
	{
		return file.generic_wstring();
	}
#else

	inline const std::string& filenameConv(const std::string& utf_str) { return utf_str; }
	inline const std::string& filepathConv(const boost::filesystem::path& file)
	{
		return file.generic_string();
	}

#endif

