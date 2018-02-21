#pragma once

#include <cstdint>
#include <cmath>

namespace OctData
{

	class HeGrayTransformXml
	{
		uint8_t* lutXML = nullptr;

		HeGrayTransformXml();
		~HeGrayTransformXml()                                    { delete[] lutXML; }

		HeGrayTransformXml(const HeGrayTransformXml&)            = delete;
		HeGrayTransformXml& operator=(const HeGrayTransformXml&) = delete;

	public:
		static HeGrayTransformXml& getInstance()                 { static HeGrayTransformXml instance; return instance; }
		static uint8_t getXmlValue(uint16_t val);

		uint8_t getValue(uint16_t val) const                     { return lutXML[val]; }
		
	};

	class HeGrayTransformVol
	{
		uint8_t* lutVol = nullptr;

		HeGrayTransformVol();
		~HeGrayTransformVol()                                    { delete[] lutVol; }

		HeGrayTransformVol(const HeGrayTransformVol&)            = delete;
		HeGrayTransformVol& operator=(const HeGrayTransformVol&) = delete;

	public:
		static HeGrayTransformVol& getInstance()                 { static HeGrayTransformVol instance; return instance; }
		static uint8_t getVolValue(uint16_t val)
		{
			const double fitVal1 = 2.38423154996781e-07;
			const double fitVal2 = 6.78813958074898e-04;
			const double fitVal3 = 4.07055468750000e+04;

			double tmpVal =  fitVal1*std::exp((static_cast<double>(val) - fitVal3)*fitVal2);
			if(tmpVal > 1.)
				tmpVal = 1.;
			tmpVal = std::pow(tmpVal, 0.25);

			return static_cast<uint8_t>(tmpVal * 255);
		}

		uint8_t getValue(uint16_t val) const                     { return lutVol[val]; }
	};

	class HeGrayTransformUFloat16
	{
		uint8_t* lut = nullptr;

		HeGrayTransformUFloat16();
		~HeGrayTransformUFloat16()                                    { delete[] lut; }

		HeGrayTransformUFloat16(const HeGrayTransformXml&)            = delete;
		HeGrayTransformUFloat16& operator=(const HeGrayTransformXml&) = delete;

	public:
		static HeGrayTransformUFloat16& getInstance()                 { static HeGrayTransformUFloat16 instance; return instance; }
		static uint8_t getUint8Value(uint16_t val);
		static double  getDoubleValue(uint16_t val);

		uint8_t getValue(uint16_t val) const                     { return lut[val]; }

	};

}
