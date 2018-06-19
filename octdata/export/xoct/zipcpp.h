#pragma once

#include<string>

typedef void* zipFile;

class ZipCpp
{
	zipFile file = nullptr;
public:
	ZipCpp(const std::string& filename);
	~ZipCpp();

	void addFile(const std::string& zipPath, const char* buff, std::size_t bufflen, bool compress = true);
	void addFile(const std::string& zipPath, const unsigned char* buff, std::size_t bufflen, bool compress = true)
	                                                               { addFile(zipPath, reinterpret_cast<const char*>(buff), bufflen, compress); }

};

