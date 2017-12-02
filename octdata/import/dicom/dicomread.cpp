#include "dicomread.h"

#ifdef USE_DCMTK

#include "../topcon/readjpeg2k.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include <algorithm>
#include <string>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmimgle/dcmimage.h>

#include "dcmtk/dcmdata/dctk.h" 
#include "dcmtk/dcmdata/dcpxitem.h"


#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>


#include<filereader/filereader.h>

#include<omp.h>
#include <cpp_framework/callback.h>

namespace bfs = boost::filesystem;


#include <boost/log/trivial.hpp>


#include<string.h>

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
	: OctFileReader({OctExtension{".dicom", ".dcm", "Dicom File"}, OctExtension("DICOMDIR", "DICOM DIR")})
	{
	}


#if false
	bool DicomRead::readFile(FileReader& filereader, OCT& oct, const FileReadOptions& /*op*/, CppFW::Callback* /*callback*/)
// 	bool readFile(const boost::filesystem::path& file, OCT& oct, const FileReadOptions& /*op*/, CppFW::Callback* /*callback*/)
	{
		const boost::filesystem::path& file = filereader.getFilepath();

		std::string ext = file.extension().generic_string();
		std::string filename = file.filename().generic_string();

		std::transform(ext.begin()     , ext.end()     , ext.begin()     , ::tolower);
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

		if(filename == "dicomdir")
			return readDicomDir(file, oct);

		if(ext != ".dicom" && ext != ".dcm")
			return false;

		if(!bfs::exists(file))
			return false;

		std::cout << "ReadDICOM: " << filename << std::endl;

		OFCondition result = EC_Normal;
		/* Load file and get pixel data element */
		DcmFileFormat dfile;
		result = dfile.loadFile(file.c_str());
		std::cout << __LINE__ << std::endl;
		if(result.bad())
			return false;

		std::cout << __LINE__ << std::endl;

		DcmDataset *data = dfile.getDataset();
		if (data == NULL)
			return false;
		std::cout << __LINE__ << std::endl;

		data->print(std::cout);

		return false;
	}

	bool DicomRead::readDicomDir(const boost::filesystem::path& file, OCT& /*oct*/)
	{
		DcmDicomDir dicomdir(file.c_str());

		// dicomdir.print(std::cout);
	//Retrieve root node
		DcmDirectoryRecord *root = &dicomdir.getRootRecord();
		//Prepare child elements
		DcmDirectoryRecord* rootTest      = new DcmDirectoryRecord(*root);
		DcmDirectoryRecord* patientRecord = nullptr;
		DcmDirectoryRecord* studyRecord   = nullptr;
		DcmDirectoryRecord* seriesRecord  = nullptr;
		DcmDirectoryRecord* image         = nullptr;

		if(rootTest == nullptr || rootTest->nextSub(patientRecord) == nullptr)
		{
			std::cout << "It looks like the selected file does not have the expected format." << std::endl;
			return false;
		}
		else
		{
			while((patientRecord = root->nextSub(patientRecord)) != nullptr)
			{
#define ReadRecord(XX) const char* str##XX; patientRecord->findAndGetString(XX, str##XX); if(str##XX) std::cout << #XX" : " << str##XX << std::endl;
// 				const char* patName;
// 				const char* patId;
// 				patientRecord->findAndGetString(DCM_PatientName, patName);
// 				patientRecord->findAndGetString(DCM_PatientID  , patId);

				ReadRecord(DCM_PatientName);
				ReadRecord(DCM_PatientID);
				ReadRecord(DCM_IssuerOfPatientID);
				ReadRecord(DCM_TypeOfPatientID);
				ReadRecord(DCM_IssuerOfPatientIDQualifiersSequence);
				ReadRecord(DCM_PatientSex);
				ReadRecord(DCM_PatientBirthDate);
				ReadRecord(DCM_PatientBirthTime);

// 				if(patName)
// 					std::cout << "patName: " << patName << std::endl;
// 				if(patId)
// 					std::cout << "patId: " << patId << std::endl;
//           if (sqi->GetItem(itemused).FindDataElement(gdcm::Tag (0x0010, 0x0010)))
//             sqi->GetItem(itemused).GetDataElement(gdcm::Tag (0x0010, 0x0010)).GetValue().Print(strm);
//           std::cout << "PATIENT NAME : " << strm.str() << std::endl;
//
				while((studyRecord = patientRecord->nextSub(studyRecord)) != nullptr)
				{
#undef ReadRecord
#define ReadRecord(XX) const char* str##XX; studyRecord->findAndGetString(XX, str##XX); if(str##XX) std::cout << #XX" : " << str##XX << std::endl;
					ReadRecord(DCM_StudyID);
					while((seriesRecord = studyRecord->nextSub(seriesRecord)) != nullptr)
					{
						while((image = seriesRecord->nextSub(image)) != nullptr)
						{
							const char *sName;
							//Retrieve the file name
							image->findAndGetString(DCM_ReferencedFileID, sName);

							//If a file is selected
							if(sName != nullptr)
							{
							//sName is the path for the file from the DICOMDIR file
							//You need to create the absolute path to use the DICOM file

							//Here you can do different tests (does the file exists ? for example)

							//Treat the dicom file
								std::cout << "sName: " << sName << std::endl;


							}
						}
					}
				}
			}
		}

		return false;
	}

#endif

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

	#if true

	bool DicomRead::readFile(FileReader& filereader, OCT& oct, const FileReadOptions& /*op*/, CppFW::Callback* callback)
// 	void ReadDICOM::readFile(const std::string& filename, CScan* cscan)
	{
		const std::string filename = filereader.getFilepath().generic_string();
		std::cout << "ReadDICOM: " << filename << std::endl;

		OFCondition result = EC_Normal;
		/* Load file and get pixel data element */
		DcmFileFormat dfile;
		result = dfile.loadFile(filename.c_str());
		if(result.bad())
			return false;

		DcmDataset *data = dfile.getDataset();
		if (data == NULL)
			return false;

		// data->print(std::cout);

		DcmElement* element = NULL;
		result = data->findAndGetElement(DCM_PixelData, element);
		if(result.bad() || element == NULL)
			return false;



		Patient& pat    = oct.getPatient(1);
		Study&   study  = pat.getStudy(1);
		Series&  series = study.getSeries(1); // TODO


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

		Uint8* pixData = nullptr;
		DcmPixelItem* pixitem = NULL;
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

			const unsigned long maxEle = dseq->card();
			// #pragma omp parallel for ordered schedule(dynamic) private(pixitem)
// 			#pragma omp parallel for ordered schedule(dynamic)
			for(unsigned long k = 1; k<maxEle; ++k)
			{
				unsigned long i = k-1;
				if(callback)
				{
					if(!callback->callback(static_cast<double>(i)/static_cast<double>(maxEle)))
						break;
				}

				std::cout << " ---- " << i << std::endl;

				char* copyPixData = nullptr;
// 	#pragma omp critical
// 				if(!pixData)
				{
					dseq->getItem(pixitem, k);
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
// 							else
// 							{
// 								copyPixData = new char[length];
// 								memcpy(copyPixData, pixData, length);
// 							}
						}
					}
				}
// 				if(copyPixData == nullptr)
// 					continue;

				// pixitem->print(std::cout);


				// Get the length of this pixel item (i.e. fragment, i.e. most of the time, the lenght of the frame)

				Uint32 length = pixitem->getLength();

				copyPixData = new char[length];
				memcpy(copyPixData, pixData, length);
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

				const char* usedPixData = nullptr; // reinterpret_cast<const char*>(copyPixData);
// 				bool deleteUsedPixData = false;

				const unsigned char jpeg2kHeader[8] =
				{
				    0x00, 0x00, 0x00, 0x0c,
				    0x6a, 0x50, 0x20, 0x20
				};



				if(memcmp(pixData, jpeg2kHeader, sizeof(jpeg2kHeader)) != 0)	// non unencrypted cirrus
				{

					std::cout << "Anormal JPEG2K, versuche umsortierung" << std::endl;
					for(char* it = copyPixData; it < copyPixData+length; it+=7)
						*it ^= 0x5a;

					const std::size_t headerpos = 3*length/5;

// 					std::size_t headerlength = 400+i;
// 					std::size_t headerlength = 305;
					std::size_t headerlength = 245;

					char* newPixData = new char[length];

					memcpy(newPixData, copyPixData, length);
					memcpy(newPixData, copyPixData+headerpos, headerlength);
					memcpy(newPixData+headerpos, copyPixData, headerlength);

					usedPixData = newPixData;
// 					std::cout << i << "data: " << length << "\t" << headerpos << "\t" << std::endl;
				}


				// ausleseversuch
				ReadJPEG2K obj;
				if(usedPixData)
				{
					if(!obj.openJpeg(usedPixData, length))
						std::cerr << "Fehler beim JPEG2K-Einlesen" << std::endl;
				}
				else
					if(!obj.openJpeg(copyPixData, length))
						std::cerr << "Fehler beim JPEG2K-Einlesen" << std::endl;

				std::cout << "*" << std::flush;


				cv::Mat gray_image;

				bool flip = true; // for Cirrus
				obj.getImage(gray_image, flip);

				putText(gray_image, boost::lexical_cast<std::string>(i), cv::Point(5, 850), cv::FONT_HERSHEY_PLAIN, 20, cv::Scalar(255));

// 				#pragma omp ordered
				{
					if(!gray_image.empty())
						series.takeBScan(new BScan(gray_image, BScan::Data()));
					else
						std::cerr << "Empty openCV image\n";
				}

// 				if(deleteUsedPixData)
					delete[] usedPixData;
				delete[] copyPixData;
			}
		}
		return true;
	}
	#endif



}
#pragma message("build with dicom support")

#else
#pragma message("build withhout dicom support")

namespace OctData
{
	DicomRead::DicomRead()
	: OctFileReader()
	{ }

	bool DicomRead::readFile(OctData::FileReader& /*filereader*/, OctData::OCT& /*oct*/, const OctData::FileReadOptions& /*op*/, CppFW::Callback* /*callback*/)
	{
		return false;
	}

}

#endif // USE_DCMTK
