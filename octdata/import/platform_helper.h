#pragma once

#include <string>

#include <boost/predef.h>
#include <boost/filesystem.hpp>


#include <codecvt>

#if BOOST_OS_WINDOWS
	#include <codecvt>

	typedef std::wstring FileString;

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

	typedef std::string FileString;

	inline const std::string& filenameConv(const std::string& utf_str) { return utf_str; }
	inline const std::string& filepathConv(const boost::filesystem::path& file)
	{
		return file.generic_string();
	}

#endif


inline std::string convertUTF16StringToUTF8(const std::u16string& u16)
{
#if BOOST_COMP_MSVC == false
	static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.to_bytes(u16);
#else
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
	std::wstring wstr(u16.begin(), u16.end());
	return convert.to_bytes(wstr);
#endif
}
