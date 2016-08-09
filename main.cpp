#include <iostream>
#include <datastruct/oct.h>
#include <octfileread.h>

int main()
{
	OctData::OCT oct = OctData::OctFileRead::openFile("test_oct.vol");
	return 0;
}
