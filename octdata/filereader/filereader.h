#pragma once

#include<boost/filesystem.hpp>

#include<opencv/cv.hpp>

namespace OctData
{
	class FileStreamInterface
	{
	public:
		FileStreamInterface() {};
		virtual ~FileStreamInterface() {};

		virtual std::streamsize read(char* dest, std::streamsize size) = 0;
		virtual void seekg(std::streamoff pos) = 0;

		virtual bool good() const = 0;
	};

	class FileReader
	{
		enum class Compressed { none, gzip };

		const boost::filesystem::path filepath;
		      boost::filesystem::path extension;

		mutable std::size_t filesize = 0;

		Compressed compressType;

		FileStreamInterface* fileStream = nullptr;

	public:
		FileReader(const boost::filesystem::path& filepath);
		~FileReader();

		const boost::filesystem::path& getFilepath()             const { return filepath; }
		const boost::filesystem::path& getExtension()            const { return extension; }


		bool openFile();
		void seekg(std::streamoff pos)                                 { fileStream->seekg(pos); }
		bool good()                                              const { return fileStream->good(); }
		std::size_t file_size()                                  const;


		template<typename T>
		void readFStream(T* dest, std::size_t num = 1) { fileStream->read(reinterpret_cast<char*>(dest), sizeof(T)*num); }

		template<typename T>
		void readCVImage(cv::Mat& image, std::size_t sizeX, std::size_t sizeY)
		{
			image = cv::Mat(static_cast<int>(sizeX), static_cast<int>(sizeY), cv::DataType<T>::type);

			const std::size_t num = sizeX*sizeY;
			fileStream->read(reinterpret_cast<char*>(image.data), sizeof(T)*num);
// 			stream.read(reinterpret_cast<char*>(image.data), num*sizeof(T));
		}
	};
}
