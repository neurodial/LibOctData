#include "filestreamdircet.h"

#include<iomanip>
#include<exception>

#include<boost/log/trivial.hpp>

#include<import/platform_helper.h>


namespace OctData
{

	FileStreamDircet::FileStreamDircet(const boost::filesystem::path& filepath)
	{
		stream.open(filepathConv(filepath), std::ios::binary | std::ios::in);
		if(!stream.good())
		{
			BOOST_LOG_TRIVIAL(error) << "Can't open vol file " << filepathConv(filepath);
			throw(""); // TODO
		}
	}
}
