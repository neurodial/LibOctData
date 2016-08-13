#include "dicomread.h"

#ifdef USE_DCMTK

#include "readjpeg2k.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


// #include <dcmtk/config/cfunix.h>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmimgle/dcmimage.h>

#include "dcmtk/dcmdata/dctk.h" 
#include "dcmtk/dcmdata/dcpxitem.h"


#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

namespace bfs = boost::filesystem;



namespace
{
	/*
	cv::Mat decodeJPEG2k(const char* pixData, std::size_t pixDataLength)
	{
		const char* usedPixData = reinterpret_cast<const char*>(pixData);
		bool deleteUsedPixData = false;

		const unsigned char jpeg2kHeader[8] =
		{
		    0x00, 0x00, 0x00, 0x0c,
		    0x6a, 0x50, 0x20, 0x20
		};

		if(memcmp(pixData, jpeg2kHeader, sizeof(jpeg2kHeader)) != 0)	// encrypted cirrus
		{
			std::cout << "Anormal JPEG2K, versuche umsortierung" << std::endl;
			for(uint8_t* it = pixData; it < pixData+pixDataLength; it+=7)
				*it ^= 0x5a;

			// umsortieren
			char* newPixData = new char[pixDataLength];
			deleteUsedPixData = true;
			std::size_t headerpos = 3*pixDataLength/5;
			std::size_t blocksize = pixDataLength/5; //  512; //
			memcpy(newPixData, pixData+headerpos, blocksize);
			memcpy(newPixData+blocksize, pixData, headerpos);
			memcpy(newPixData+headerpos+blocksize, pixData+headerpos+blocksize, pixDataLength-headerpos-blocksize);

			usedPixData = newPixData;

/ *
			// test datei schreiben
			std::string number = boost::lexical_cast<std::string>(i);
			std::fstream outFileTest("jp2/" + number + ".jp2", std::ios::binary | std::ios::out);
			outFileTest.write(reinterpret_cast<const char*>(usedPixData), length);
			outFileTest.close();
* /
		}

		if(memcmp(pixData, jpeg2kHeader, sizeof(jpeg2kHeader)) != 0)	// decrypted cirrus data not successful
			return cv::Mat();


		// ausleseversuch
		ReadJPEG2K obj;
		if(!obj.openJpeg(usedPixData, pixDataLength))
			std::cerr << "Fehler beim JPEG2K-Einlesen" << std::endl;

		std::cout << "*" << std::flush;


		cv::Mat gray_image;

		bool flip = true; // for Cirrus
		obj.getImage(gray_image, flip);

		if(deleteUsedPixData)
			delete[] usedPixData;

		return gray_image;
	}
	*/
}


namespace OctData
{
	DicomRead::DicomRead()
	: OctFileReader(OctExtension("dicom", "Dicom File"))
	{ }

	bool DicomRead::readFile(const boost::filesystem::path& file, OCT& /*oct*/)
	{
		if(file.extension() != ".dicom")
			return false;

		if(!bfs::exists(file))
			return false;

		return false;
	}


	DicomRead* DicomRead::getInstance()
	{
		static DicomRead instance; return &instance;
	}


	#if false
	void ReadDICOM::readFile(const std::string& filename, CScan* cscan)
	{

		if(!cscan)
			return;

		bfs::path xmlfile(filename);
		if(!bfs::exists(xmlfile))
			return;


		/*
		DcmDicomDir dicomdir(filename.c_str());
		dicomdir.print(std::cout);
		dicomdir.getRootRecord();
		*/



		//Open the DICOMDIR File
		// QString DICOMDIR_folder = "C:/Folder1/Folder2";
		const char *fileName = filename.c_str();
		DcmDicomDir dicomdir(fileName);


		dicomdir.print(std::cout);

		//Retrieve root node
		DcmDirectoryRecord *root = &dicomdir.getRootRecord();
		//Prepare child elements
		DcmDirectoryRecord *rootTest = new DcmDirectoryRecord(*root);
		DcmDirectoryRecord *PatientRecord = NULL;
		DcmDirectoryRecord *StudyRecord = NULL;
		DcmDirectoryRecord *SeriesRecord = NULL;
		DcmDirectoryRecord *image = NULL;

		if(rootTest == NULL || rootTest->nextSub(PatientRecord) == NULL)
			std::cout << "It looks like the selected file does not have the expected format." << std::endl;
		else
		{
			while((PatientRecord = root->nextSub(PatientRecord)) != NULL)
			{
				while((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL)
				{
					while((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL)
					{
						while((image = SeriesRecord->nextSub(image)) != NULL)
						{
							const char *sName;
							//Retrieve the file name
							image->findAndGetString(DCM_ReferencedFileID, sName);

							//If a file is selected
							if(sName != "")
							{
							//sName is the path for the file from the DICOMDIR file
							//You need to create the absolute path to use the DICOM file

							//Here you can do different tests (does the file exists ? for example)

							//Treat the dicom file
								std::cout << sName << std::endl;


							}
						}
					}
				}
			}
		}

		return;

		// http://support.dcmtk.org/redmine/projects/dcmtk/wiki/Howto_AccessingCompressedData
		// https://stackoverflow.com/questions/28219632/dicom-accessing-compressed-data-dcmtkt

		DicomImage DCM_image(filename.c_str());

		int depth      = DCM_image.getDepth();
		int n_channels = 1;

		std::cout << "depth: " << depth << std::endl;


		for(std::size_t i = 0; i<DCM_image.getFrameCount(); ++i)
		{
			cv::Mat image(DCM_image.getHeight(), DCM_image.getWidth(), CV_MAKETYPE(depth, n_channels), const_cast<void*>(DCM_image.getOutputData(0, i)));

			cv::Mat dest;
			image.convertTo(dest, CV_32FC1, 1/static_cast<double>(1 << 16), 0);

			cscan->takeBScan(new BScan(dest, BScan::Data()));
		}

	}
	#endif

	#if false
	void ReadDICOM::readFile(const std::string& filename, CScan* cscan)
	{
		std::cout << "ReadDICOM: " << filename << std::endl;

		OFCondition result = EC_Normal;
		/* Load file and get pixel data element */
		DcmFileFormat dfile;
		result = dfile.loadFile(filename.c_str());
		if(result.bad())
			return;

		DcmDataset *data = dfile.getDataset();
		if (data == NULL)
			return;

		// data->print(std::cout);

		DcmElement* element = NULL;
		result = data->findAndGetElement(DCM_PixelData, element);
		if(result.bad() || element == NULL)
			return;

		DcmPixelData *dpix = NULL;
		dpix = OFstatic_cast(DcmPixelData*, element);
		/* Since we have compressed data, we must utilize DcmPixelSequence
			in order to access it in raw format, e. g. for decompressing it
			with an external library.
		*/

		// dpix->print(std::cout);

		DcmPixelSequence *dseq = NULL;
		E_TransferSyntax xferSyntax = EXS_Unknown;
		const DcmRepresentationParameter *rep = NULL;

		// Find the key that is needed to access the right representation of the data within DCMTK
		dpix->getOriginalRepresentationKey(xferSyntax, rep);

		// Access original data representation and get result within pixel sequence
		result = dpix->getEncapsulatedRepresentation(xferSyntax, rep, dseq);

		if(result == EC_Normal)
		{

			// Access first frame (skipping offset table)

	// 		Uint32 u;
	// 		Uint32 c;
	// 		char buffer[50000];
	// 		OFString str;
			// dseq->getUncompressedFrameSize(data, u);
			// dseq->getUncompressedFrame(data, 1, c, buffer, 50000, str);
			// std::cout << u << " - " << c << " - " << dseq->card() << std::endl;
			// dseq->print(std::cout);

			const int maxEle = dseq->card();
			// #pragma omp parallel for ordered schedule(dynamic) private(pixitem)
			#pragma omp parallel for ordered schedule(dynamic)
			for(int i = 1; i<maxEle; ++i)
			{

				char* copyPixData = nullptr;
				Uint8* pixData = NULL;
				DcmPixelItem* pixitem = NULL;
	#pragma omp critical
				{
					dseq->getItem(pixitem, i);
					if(pixitem != NULL)
					{
						Uint32 length = pixitem->getLength();
						if(length == 0)
						{
							std::cerr << "unexpected pixitem lengt 0, ignore item\n";
						}
						else
						{
							result = pixitem->getUint8Array(pixData);
							if(result != EC_Normal)
							{
								std::cout << "defect Pixdata" << std::endl;
							}
							else
							{
								copyPixData = new char[length];
								memcpy(copyPixData, pixData, length);
							}
						}
					}
				}
				if(copyPixData == nullptr)
					continue;

				// pixitem->print(std::cout);


				// Get the length of this pixel item (i.e. fragment, i.e. most of the time, the lenght of the frame)

				Uint32 length = pixitem->getLength();
	// 			if(length == 0)
	// 			{
	// 				std::cerr << "unexpected pixitem lengt 0, ignore item\n";
	// 				continue;
	// 			}
	//
	// 			// Finally, get the compressed data for this pixel item
	// 			result = pixitem->getUint8Array(pixData);
	// 			if(result != EC_Normal)
	// 			{
	// 				std::cout << "defect Pixdata" << std::endl;
	// 				continue;
	// 			}

				// Do something useful with pixData...

				const char* usedPixData = reinterpret_cast<const char*>(copyPixData);
				bool deleteUsedPixData = false;

				const unsigned char jpeg2kHeader[8] =
				{
				    0x00, 0x00, 0x00, 0x0c,
				    0x6a, 0x50, 0x20, 0x20
				};


				if(memcmp(pixData, jpeg2kHeader, sizeof(jpeg2kHeader)) != 0)	// non unencrypted cirrus
				{
					std::cout << "Anormal JPEG2K, versuche umsortierung" << std::endl;
					for(uint8_t* it = pixData; it < pixData+length; it+=7)
						*it ^= 0x5a;

					// umsortieren
					char* newPixData = new char[length];
					deleteUsedPixData = true;
					std::size_t headerpos = 3*length/5;
					std::size_t blocksize = length/5; //  512; //
					memcpy(newPixData, pixData+headerpos, blocksize);
					memcpy(newPixData+blocksize, pixData, headerpos);
					memcpy(newPixData+headerpos+blocksize, pixData+headerpos+blocksize, length-headerpos-blocksize);

					usedPixData = newPixData;


					// test datei schreiben
					std::string number = boost::lexical_cast<std::string>(i);
					std::fstream outFileTest("jp2/" + number + ".jp2", std::ios::binary | std::ios::out);
					outFileTest.write(reinterpret_cast<const char*>(usedPixData), length);
					outFileTest.close();
				}
	/*
				for(uint8_t* it = pixData; it < pixData+length; it+=7)
				{
					*it ^= 0x5a;
				}*/


				// umsortieren
	// 			uint8_t* tempData = new uint8_t[length];
	// 			std::size_t headerpos = 3*length/5;
	// 			std::size_t blocksize = 1024; // length/5;
	// 			memcpy(tempData, pixData+headerpos, blocksize);
	// 			memcpy(tempData+blocksize, pixData, headerpos);
	// 			memcpy(tempData+headerpos+blocksize, pixData+headerpos+blocksize, length-headerpos-blocksize);
	/*
				std::cout << "It: " << i << "\tlänge: " << length << std::endl;
				std::cout << "\tHeader-Pos: " << headerpos << " - " << (length % 5)*3 << std::endl;*/

		// 		memcpy(tempData, pixData+blocksize*3, blocksize*2);
		// 		memcpy(tempData+blocksize*2, pixData, blocksize*3);



	// 			std::fstream outFile("jp2/" + number + "_umsort.jp2", std::ios::binary | std::ios::out);
	// 			outFile.write(reinterpret_cast<char*>(tempData), length);
	// 			outFile.close();



				// ausleseversuch
				ReadJPEG2K obj;
				if(!obj.openJpeg(usedPixData, length))
					std::cerr << "Fehler beim JPEG2K-Einlesen" << std::endl;

				std::cout << "*" << std::flush;


				cv::Mat gray_image;

				bool flip = true; // for Cirrus
				obj.getImage(gray_image, flip);

				#pragma omp ordered
				{
					if(!gray_image.empty())
						cscan->takeBScan(new BScan(gray_image, BScan::Data()));
					else
						std::cerr << "Empty openCV image\n";
				}

				if(deleteUsedPixData)
					delete[] usedPixData;
				delete[] copyPixData;
			}
		}
	}
	#endif



}

#else

namespace OctData
{
	DicomRead::DicomRead()
	: OctFileReader()
	{ }

	bool DicomRead::readFile(const boost::filesystem::path&, OCT&)
	{

	}


	DicomRead* DicomRead::getInstance()
	{
		static DicomRead instance; return &instance;
	}
}

#endif // USE_DCMTK
