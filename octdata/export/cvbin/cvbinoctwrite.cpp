/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cvbinoctwrite.h"

#include <datastruct/oct.h>
#include <datastruct/coordslo.h>
#include <datastruct/sloimage.h>
#include <datastruct/bscan.h>

#include <opencv2/opencv.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <filewriteoptions.h>


#include <boost/log/trivial.hpp>

#include <oct_cpp_framework/cvmat/cvmattreestruct.h>
#include <oct_cpp_framework/cvmat/cvmattreestructextra.h>
#include <oct_cpp_framework/cvmat/cvmattreegetset.h>
#include <oct_cpp_framework/cvmat/treestructbin.h>
#include <oct_cpp_framework/callback.h>
#include <octfileread.h>


namespace bfs = boost::filesystem;


namespace OctData
{
	namespace
	{
		// general export methods
		void writeSlo(CppFW::CVMatTree& sloNode, const SloImage& slo)
		{
			CppFW::SetToCVMatTree sloWriter(sloNode);
			slo.getSetParameter(sloWriter);
			sloNode.getDirNode("img").getMat() = slo.getImage();
		}

		void writeImage(CppFW::CVMatTree& tree, const cv::Mat& image, const std::string& nodeName)
		{
			if(image.empty())
				return;

			CppFW::CVMatTree& imgNode = tree.getDirNode(nodeName);
			imgNode.getMat() = image;
		}

		void writeBScan(CppFW::CVMatTree& seriesNode, const BScan* bscan)
		{
			if(!bscan)
				return;

			CppFW::CVMatTree& bscanNode = seriesNode.newListNode();
			writeImage(bscanNode, bscan->getImage()     , "img"     );
			writeImage(bscanNode, bscan->getAngioImage(), "angioImg");

			CppFW::CVMatTree& bscanDataNode = bscanNode.getDirNode("data");
			CppFW::SetToCVMatTree bscanWriter(bscanDataNode);
			bscan->getSetParameter(bscanWriter);

			CppFW::CVMatTree& bscanSegNode = bscanNode.getDirNode("segmentations");

			for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
			{
				const Segmentationlines::Segmentline& seg = bscan->getSegmentLine(type);
				if(!seg.empty())
					bscanSegNode.getDirNode(Segmentationlines::getSegmentlineName(type)).getMat() = cv::Mat(1, static_cast<int>(seg.size()), cv::DataType<Segmentationlines::SegmentlineDataType>::type, const_cast<Segmentationlines::SegmentlineDataType*>(seg.data())).clone();
			}
		}

		// deep file format (support many scans per file, tree structure)
		template<typename S>
		bool writeStructure(CppFW::CVMatTree& tree, const S& structure)
		{
			bool result = true;
			CppFW::CVMatTree& dataNode    = tree.getDirNode("data");
// 			CppFW::CVMatTree& subNodeList = tree.getDirNode("Sub"); // TODO
			CppFW::SetToCVMatTree structureWriter(dataNode);
			structure.getSetParameter(structureWriter);

			for(typename S::SubstructurePair const& subStructPair : structure)
			{
				CppFW::CVMatTree& subNode = tree.getDirNode("id_" + boost::lexical_cast<std::string>(subStructPair.first));
				result &= writeStructure(subNode, *subStructPair.second);
			}
			return result;
		}


		template<>
		bool writeStructure<Series>(CppFW::CVMatTree& tree, const Series& series)
		{
			CppFW::CVMatTree& seriesDataNode = tree.getDirNode("data");
			CppFW::SetToCVMatTree seriesWriter(seriesDataNode);
			series.getSetParameter(seriesWriter);


			writeSlo(tree.getDirNode("slo"), series.getSloImage());

			CppFW::CVMatTree& seriesNode = tree.getDirNode("bscans");

			for(BScan* bscan : series.getBScans())
				writeBScan(seriesNode, bscan);

			return true;
		}


		// flat file format (only one scan per file)
		bool writeFlatFile(CppFW::CVMatTree& octtree, const Patient& pat, const Study& study, const Series& series)
		{
			CppFW::CVMatTree& patDataNode    = octtree.getDirNode("patientData");
			CppFW::CVMatTree& studyDataNode  = octtree.getDirNode("studyData"  );
			CppFW::CVMatTree& seriesDataNode = octtree.getDirNode("seriesData" );

			CppFW::CVMatTreeExtra::setCvScalar(patDataNode   , "ID", pat   .getInternalId());
			CppFW::CVMatTreeExtra::setCvScalar(studyDataNode , "ID", study .getInternalId());
			CppFW::CVMatTreeExtra::setCvScalar(seriesDataNode, "ID", series.getInternalId());

			CppFW::SetToCVMatTree patientWriter(patDataNode);
			pat.getSetParameter(patientWriter);

			CppFW::SetToCVMatTree studyWriter(studyDataNode);
			study.getSetParameter(studyWriter);

			CppFW::SetToCVMatTree seriesWriter(seriesDataNode);
			series.getSetParameter(seriesWriter);

			writeSlo(octtree.getDirNode("slo"), series.getSloImage());

			CppFW::CVMatTree& seriesNode = octtree.getDirNode("serie");
			for(BScan* bscan : series.getBScans())
				writeBScan(seriesNode, bscan);

			return true;
		}

		bool writeFlatFile(CppFW::CVMatTree& octtree, const OCT& oct)
		{
			OCT::SubstructureCIterator pat = oct.begin();
			const Patient* p = pat->second;
			if(!p)
				return false;

			Patient::SubstructureCIterator study = p->begin();
			const Study* s = study->second;
			if(!s)
				return false;

			Study::SubstructureCIterator series = s->begin();
			const Series* ser = series->second;

			if(!ser)
				return false;

			return writeFlatFile(octtree, *p, *s, *ser);
		}

	}

	void setStringOpt(CppFW::CVMatTree& tree, const char* nodeName, const std::string& value)
	{
		if(!value.empty())
			tree.getDirNode(nodeName).getString() = value;
	}

	bool CvBinOctWrite::writeFile(const boost::filesystem::path& file, const OCT& oct, const FileWriteOptions& opt)
	{
		CppFW::CVMatTree octtree;

		bool result;
		if(opt.octBinFlat)
			result = writeFlatFile(octtree, oct);
		else
			result = writeStructure(octtree, oct);

// 		if(result)
		result &= CppFW::CVMatTreeStructBin::writeBin(file.generic_string(), octtree);

		return result;
	}

}
