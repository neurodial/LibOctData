#ifndef READJPEG2K_H
#define READJPEG2K_H

#include <cstdint>
#include <string>

typedef void* opj_stream_t;

namespace cv { class Mat; }

class ReadJPEG2K
{
	void* memoryStream;

	opj_stream_t* opjStreamCreateDefaultMemoryStream(const char* data, std::size_t dataSize);

	void* imageVoid = nullptr;

	template<typename T>
	bool copyMatrix(cv::Mat& matrix, bool flip);

public:
	ReadJPEG2K();
	~ReadJPEG2K();

	
	bool openJpeg(const char* data, std::size_t dataSize);

	bool getImage(cv::Mat& image, bool flip);
	
};

#endif // READJPEG2K_H
