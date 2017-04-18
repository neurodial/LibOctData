#include <iostream>
#include <datastruct/oct.h>
#include <octfileread.h>

int main()
{
	const char* filename = "test_stack.tif";

	std::cout << "open: " << filename << std::endl;

	OctData::OCT oct = OctData::OctFileRead::openFile(filename);

	return 0;
}
