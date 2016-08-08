#include "octfileread.h"

#include <datastruct/oct.h>

OctData::OctFileRead::OctFileRead()
{
	extensions.push_back(OctExtension("xml","Heidelberg Engineering OCT XML"));
}


OctData::OctFileRead::~OctFileRead()
{

}


OctData::OCT&& OctData::OctFileRead::openFile(const std::string& /*filename*/)
{
	OctData::OCT oct;
	return std::move(oct);
}


