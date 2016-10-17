#include "readjpeg2k.h"

// https://webcache.googleusercontent.com/search?q=cache:da8jZjV92m4J:https://groups.google.com/d/topic/openjpeg/8cebr0u7JgY%2Bopenjpeg+from+buffer&hl=de&ct=clnk
// https://groups.google.com/d/topic/openjpeg/8cebr0u7JgY

#include <cstring>
#include <cstdint>

#include <cassert>


#include <openjpeg-2.1/openjpeg.h>
#include <opencv/cv.h>

// TODO
#include <iostream>


extern "C" {

	void opj_error_callback(const char* msg, void* /*usr*/)
	{
		std::cerr << "Err: OpenJPEG: " << msg << std::endl;
	}

	void opj_warning_callback(const char* msg, void*)
	{
		std::cout << "Wrn: OpenJPEG: " << msg << std::endl;
	}

	void opj_info_callback(const char* msg, void*)
	{
		std::cout << "Info: OpenJPEG: " << msg << std::endl;
	}
}


namespace
{

	// These routines are added to use memory instead of a file for input and output.
		//Structure need to treat memory as a stream.
	typedef struct
	{
		const OPJ_UINT8* pData; //Our data.
		OPJ_SIZE_T dataSize; //How big is our data.
		OPJ_SIZE_T offset; //Where are we currently in our data.
	} opj_memory_stream;

	opj_memory_stream* getStream(void* p) { return reinterpret_cast<opj_memory_stream*>(p); }
	opj_image*& getJp2tImage(void*& p)    { return reinterpret_cast<opj_image*&>(p); }


	//This will read from our memory to the buffer.
	static OPJ_SIZE_T opj_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = reinterpret_cast<opj_memory_stream*>(p_user_data);//Our data.
		OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.
		//Check if the current offset is outside our data buffer.
		if (l_memory_stream->offset >= l_memory_stream->dataSize)
			return static_cast<OPJ_SIZE_T>(-1);
		
		//Check if we are reading more than we have.
		if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
			l_nb_bytes_read = l_memory_stream->dataSize - l_memory_stream->offset;//Read all we have.
		
		//Copy the data to the internal buffer.
		memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);
		l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.
		return l_nb_bytes_read;
	}
/*
	//This will write from the buffer to our memory.
	static OPJ_SIZE_T opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = reinterpret_cast<opj_memory_stream*>(p_user_data);//Our data.
		OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.
		//Check if the current offset is outside our data buffer.
		if(l_memory_stream->offset >= l_memory_stream->dataSize)
			return static_cast<OPJ_SIZE_T>(-1);
		
		//Check if we are write more than we have space for.
		if(p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
			l_nb_bytes_write = l_memory_stream->dataSize - l_memory_stream->offset;//Write the remaining space.
		
		//Copy the data from the internal buffer.
		std::memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);
		l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.
		return l_nb_bytes_write;
	}
*/

	//Moves the pointer forward, but never more than we have.
	static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = reinterpret_cast<opj_memory_stream*>(p_user_data);
		OPJ_SIZE_T l_nb_bytes;
		if(p_nb_bytes < 0) return -1;//No skipping backwards.
		l_nb_bytes = static_cast<OPJ_SIZE_T>(p_nb_bytes);//Allowed because it is positive.
		// Do not allow jumping past the end.
		if(l_nb_bytes >l_memory_stream->dataSize - l_memory_stream->offset)
			l_nb_bytes = l_memory_stream->dataSize - l_memory_stream->offset;//Jump the max.

		//Make the jump.
		l_memory_stream->offset += l_nb_bytes;

		//Returm how far we jumped.
		return l_nb_bytes;
	}

	//Sets the pointer to anywhere in the memory.
	static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
	{
		opj_memory_stream* l_memory_stream = reinterpret_cast<opj_memory_stream*>(p_user_data);

		if(p_nb_bytes < 0)
			return OPJ_FALSE;//No before the buffer.
		if(p_nb_bytes > static_cast<OPJ_OFF_T>(l_memory_stream->dataSize))
			return OPJ_FALSE;//No after the buffer.
			
		l_memory_stream->offset = static_cast<OPJ_SIZE_T>(p_nb_bytes);//Move to new position.
		return OPJ_TRUE;
	}

	//The system needs a routine to do when finished, the name tells you what I want it to do.
	static void opj_memory_stream_do_nothing(void * p_user_data)
	{
		OPJ_ARG_NOT_USED(p_user_data);
	}

	//Create a stream to use memory as the input or output.
	opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream)
	{
		opj_stream_t* l_stream;
		
		if(!(l_stream = opj_stream_default_create(p_is_read_stream)))
			return (NULL);
		//Set how to work with the frame buffer.
		if(p_is_read_stream)
			opj_stream_set_read_function(l_stream, opj_memory_stream_read);
		else
			assert(false);
			// opj_stream_set_write_function(l_stream, opj_memory_stream_write);

		opj_stream_set_seek_function(l_stream, opj_memory_stream_seek);
		opj_stream_set_skip_function(l_stream, opj_memory_stream_skip);
		opj_stream_set_user_data(l_stream, p_memoryStream, opj_memory_stream_do_nothing);
		opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);

		return l_stream;
	}
}


opj_stream_t* ReadJPEG2K::opjStreamCreateDefaultMemoryStream(const char* data, std::size_t dataSize)
{
	opj_memory_stream* memStream = getStream(memoryStream);
	memStream->dataSize = dataSize;
	memStream->pData    = reinterpret_cast<const OPJ_UINT8*>(data);
	memStream->offset   = 0;
	
	return opj_stream_create_default_memory_stream(memStream, true);
}

/*
namespace
{
	OPJ_COLOR_SPACE getOpenJpegColorSpace(const std::string& photometricInterpretation)
	{
		if(photometricInterpretation == "RGB")
			return OPJ_CLRSPC_SRGB;
		else if(photometricInterpretation == "MONOCHROME1" || photometricInterpretation == "MONOCHROME2")
			return OPJ_CLRSPC_GRAY;
		else if(photometricInterpretation == "PALETTE COLOR")
			return OPJ_CLRSPC_GRAY;
		else if (photometricInterpretation == "YBR_FULL" || photometricInterpretation == "YBR_FULL_422" || photometricInterpretation == "YBR_PARTIAL_422")
			return OPJ_CLRSPC_SYCC;
		else
			return OPJ_CLRSPC_UNKNOWN;
	}
}
*/


ReadJPEG2K::ReadJPEG2K()
: memoryStream(new opj_memory_stream)
{

}


ReadJPEG2K::~ReadJPEG2K()
{
	delete getStream(memoryStream);
	opj_image_destroy(getJp2tImage(imageVoid));
}


bool ReadJPEG2K::openJpeg(const char* data, std::size_t dataSize)
{
	opj_stream_t* stream = opjStreamCreateDefaultMemoryStream(data, dataSize);
	
	/*
	opj_image_t* image = nullptr;


	opj_image_cmptparm_t cmptparm[3];
	opj_cparameters_t eparams;  // compression parameters
	// opj_event_mgr_t event_mgr;  // event manager
	// opj_cinfo_t* cinfo = NULL;  // handle to a compressor
	opj_codec_t* l_codec = NULL;                  // Handle to a decompressor
	opj_image_t *image = NULL;
	// opj_cio_t *cio = NULL;
	
	
	memset(&cmptparm[0], 0, sizeof(opj_image_cmptparm_t) * 3);
	for(int i = 0; i < dcmData->SamplesPerPixel; i++) {
		cmptparm[i].bpp  = dcmData->BitsAllocated;
		cmptparm[i].prec = dcmData->BitsStored;
		cmptparm[i].sgnd = dcmData->IsSigned ? 1 : 0;
		cmptparm[i].dx   = eparams.subsampling_dx;
		cmptparm[i].dy   = eparams.subsampling_dy;
		cmptparm[i].h    = dcmData->ImageHeight;
		cmptparm[i].w    = dcmData->ImageWidth;
	}

	
	image = opj_image_create(dcmData->SamplesPerPixel, &cmptparm[0], getOpenJpegColorSpace(dcmData.PhotometricInterpretation));

	opj_dparameters_t parameters;
	opj_set_default_decoder_parameters(&parameters);
	
	parameters.cp_layer = 0;
	parameters.cp_reduce = 0;
	parameters.DA_x1 = 1024;
	parameters.DA_y1 = 512;
//	parameters.
	

	
	l_codec = opj_create_decompress(OPJ_CODEC_J2K);
	if(!opj_setup_decoder(l_codec, &parameters))
	{
		opj_destroy_codec(l_codec);
		std::cout << "opj_setup_decoder" << std::endl;;
		return;
	}
	
	*/
	
	opj_codec_t* l_codec = NULL;                  // Handle to a decompressor
	opj_image*& image = getJp2tImage(imageVoid);
	if(image != nullptr)
	{
		opj_image_destroy(image);
		image = nullptr;
	}
	
	opj_dparameters_t parameters;
	opj_set_default_decoder_parameters(&parameters);
	
	parameters.cp_layer = 0;
	parameters.cp_reduce = 0;
	parameters.m_verbose = true;
	parameters.jpwl_exp_comps = 1;
	
	l_codec = opj_create_decompress(OPJ_CODEC_JP2);
	if(l_codec == nullptr)
		return false;
	
	// bool infoH = opj_set_info_handler   (l_codec, opj_info_callback   , 0);
	/* bool warnH = */ opj_set_warning_handler(l_codec, opj_warning_callback, 0);
	/* bool erroH = */ opj_set_error_handler  (l_codec, opj_error_callback  , 0);

		/* Setup the decoder decoding parameters using user parameters */
	if(!opj_setup_decoder(l_codec, &parameters))
	{
		std::cerr << "ERROR -> opj_compress: failed to setup the decoder\n";
		opj_stream_destroy(stream);
		opj_destroy_codec(l_codec);
		return false;
	}

	/* Read the main header of the codestream and if necessary the JP2 boxes*/
	if(! opj_read_header(stream, l_codec, &image)){
		std::cerr << "ERROR -> opj_decompress: failed to read the header\n";
		opj_stream_destroy(stream);
		opj_destroy_codec(l_codec);
		opj_image_destroy(image);
		return false;
	}

	// std::cout << infoH << warnH << erroH << std::endl;
	
// 	unsigned int l_tile_index     ;
// 	unsigned int l_data_size      ;
// 	int l_current_tile_x0;
// 	int l_current_tile_y0;
// 	int l_current_tile_x1;
// 	int l_current_tile_y1;
// 	unsigned int l_nb_comps       ;
// 	int l_go_on          ;
//
// 	opj_read_tile_header(
//             l_codec,
//             stream,
//             &l_tile_index,
//             &l_data_size,
//             &l_current_tile_x0,
//             &l_current_tile_y0,
//             &l_current_tile_x1,
//             &l_current_tile_y1,
//             &l_nb_comps,
//             &l_go_on);
//
// 	std::cout << "l_tile_index      : " << l_tile_index      << '\n'
// 	          << "l_data_size       : " << l_data_size       << '\n'
// 	          << "l_current_tile_x0 : " << l_current_tile_x0 << '\n'
// 	          << "l_current_tile_y0 : " << l_current_tile_y0 << '\n'
// 	          << "l_current_tile_x1 : " << l_current_tile_x1 << '\n'
// 	          << "l_current_tile_y1 : " << l_current_tile_y1 << '\n'
// 	          << "l_nb_comps        : " << l_nb_comps        << '\n'
// 	          << "l_go_on           : " << l_go_on           << '\n'<< std::endl;



	/*      */

	bool result = opj_decode(l_codec, stream, image);

	opj_stream_destroy(stream);
	opj_destroy_codec(l_codec);

	return result;
	/*
	std::cout << "opj_decode: " << opj_decode(l_codec, stream, image) << std::endl;;
	std::cout << "image: " << image << std::endl;;*/

}


bool ReadJPEG2K::getImage(cv::Mat& cvImage, bool flip)
{
	opj_image* image = getJp2tImage(imageVoid);

	if(image == 0)
		return false;

	const int depth    = image->comps[0].prec;

	switch(depth)
	{
		case 8:
			return copyMatrix<uint8_t>(cvImage,  flip);
		case 16:
			return copyMatrix<uint16_t>(cvImage, flip);
		default:
			std::cerr << __FUNCTION__ << ":" << __LINE__ << ": unhandelt format: " << depth << "\n";
			return false;
	}


	return true;
}

template<typename T>
bool ReadJPEG2K::copyMatrix(cv::Mat& cvImage, bool flip)
{
	opj_image* image = getJp2tImage(imageVoid);

	if(image->numcomps > 4)
		std::cerr << "More than 4 channels, ignore";

	const int channels = image->numcomps>4?4:image->numcomps;
	const int type     = CV_MAKETYPE(cv::DataType<T>::type, channels);

	const int width  = static_cast<int>(image->comps[0].w);
	const int height = static_cast<int>(image->comps[0].h);

	// flip the image
	int rows = height;
	int cols = width;
	int lineAdd = 1;
	int rowAdd  = width;
	if(flip)
	{
		rows = width;
		cols = height;
		lineAdd = width;
		rowAdd  = 1;
	}


	cvImage.create(rows, cols, type);


	OPJ_INT32* dataIt[4];

//     int adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
//     int adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
//     int adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);

	for(int i = 0; i < cvImage.rows; i++)
	{
		for(int c=0; c<channels; ++c)
			dataIt[c] = image->comps[0].data + i*rowAdd;

		T* mi = cvImage.ptr<T>(i);
		for(int j = 0; j < cvImage.cols; j++)
		{
			for(int c = 0; c < channels; ++c)
			{
				*mi = static_cast<T>(*(dataIt[c]));
				++mi;
				dataIt[c] += lineAdd;
			}
		}
	}

	return true;
}

