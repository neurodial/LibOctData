#pragma once

#include<type_traits>

#include<boost/filesystem.hpp>
#include<boost/endian/arithmetic.hpp>

#include<opencv/cv.hpp>


#include<iostream>

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

		template<typename T>
		void readFStreamBigInt(T* dest, std::size_t num, std::true_type)
		{
			fileStream->read(reinterpret_cast<char*>(dest), sizeof(T)*num);
			for(std::size_t i = 0; i<num; ++i)
				boost::endian::big_to_native_inplace(dest[i]);
		}


		void convertFloatType(float & value) { boost::endian::big_to_native_inplace(*(reinterpret_cast<uint32_t*>(&value))); }
		void convertFloatType(double& value) { boost::endian::big_to_native_inplace(*(reinterpret_cast<uint64_t*>(&value))); }

		template<typename T>
		void readFStreamBigInt(T* dest, std::size_t num, std::false_type) // TODO: correct endian for floating types
		{
			readFStream(dest, num);
			for(std::size_t i = 0; i<num; ++i)
				convertFloatType(dest[i]);
		}


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
		void readFStreamBig(T* dest, std::size_t num = 1) { readFStreamBigInt<T>(dest, num, std::is_integral<T>()); }


		void readString(std::string& dest, std::size_t num = 1)
		{
			dest.resize(num);
			fileStream->read(const_cast<char*>(dest.data()), sizeof(std::string::value_type)*num); // TODO: c++17: remove const_cast
		}


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
