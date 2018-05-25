#pragma once
#include <string>

#include "../octfilereader.h"

namespace OctData
{
	template<typename T> struct GIPLFilterType;
	template<> struct GIPLFilterType<uint8_t >{ constexpr static const int typeId = 8 ; };
	template<> struct GIPLFilterType<uint16_t>{ constexpr static const int typeId = 15; };

	class FileReader;

	class GIPLRead : public OctFileReader
	{
	public:
		class GiplHeader
		{
		public:
			struct RawData
			{
				uint16_t    sizes        [ 4];
				uint16_t    image_type       ;
				float       scales       [ 4];
				std::string patient          ;
				float       matrix       [20];
				uint8_t     orientation      ;
				uint8_t     par2             ;
				double      voxmin           ;
				double      voxmax           ;
				double      origin       [ 4];
				float       pixval_offset    ;
				float       pixval_cal       ;
				float       interslicegap    ;
				float       user_def2        ;
				uint32_t    magic_number     ;
			};

			GiplHeader() = default;
			GiplHeader(const RawData& data) : header(data) {}

			uint16_t getSizeX () const                                     { return header.sizes[0]  ; }
			uint16_t getSizeY () const                                     { return header.sizes[1]  ; }
			uint16_t getSizeZ () const                                     { return header.sizes[2]  ; }
			uint16_t getType  () const                                     { return header.image_type; }

			void print(std::ostream& stream) const;
			void readInfo(FileReader& filereader);
			bool numberCheck() const
			{
				return header.magic_number == giplMagicNumber;
			}

			template<typename T>
			void writeHeader(T& writer) const
			{
				getSetParameter(writer, *this);
			}

			constexpr static const uint32_t giplMagicNumber = 4026526128;

		private:
			RawData header;

			template<typename T, typename VI>
			static void getSetParameter(T& getset, VI& data)
			{
				getset.template op<uint16_t>("sizes        ", data.header.sizes        , 4);
				getset.template op<uint16_t>("image_type   ", data.header.image_type   , 1);
				getset.template op<float   >("scales       ", data.header.scales       , 4);
				getset.template op<char    >("patient      ", data.header.patient      ,80);
				getset.template op<float   >("matrix       ", data.header.matrix       ,20);
				getset.template op<uint8_t >("orientation  ", data.header.orientation  , 1);
				getset.template op<uint8_t >("par2         ", data.header.par2         , 1);
				getset.template op<double  >("voxmin       ", data.header.voxmin       , 1);
				getset.template op<double  >("voxmax       ", data.header.voxmax       , 1);
				getset.template op<double  >("origin       ", data.header.origin       , 4);
				getset.template op<float   >("pixval_offset", data.header.pixval_offset, 1);
				getset.template op<float   >("pixval_cal   ", data.header.pixval_cal   , 1);
				getset.template op<float   >("interslicegap", data.header.interslicegap, 1);
				getset.template op<float   >("user_def2    ", data.header.user_def2    , 1);
				getset.template op<uint32_t>("magic_number ", data.header.magic_number , 1);
			}
		};

		GIPLRead();

	    virtual bool readFile(FileReader& filereader, OCT& oct, const FileReadOptions& op, CppFW::Callback* callback) override;
	};
}

