# LibOctData

a library for reading different OCT file formats

It supports the following formats

  * Heidelberg Engineering Xml File (xml + tiff)
  * Heidelberg Engineering Raw File (vol)
  * nativ Heidelberg Engineering format (E2E, sdb) (with LibE2E)
  * Topcon (fda) (thanks to the uoct project for documentation)
  * Bioptigen Oct file (OCT)
  * Cirrus Dicom (dicom) (highly experimental, mostly with huge artefacts in the images)
  * Cirrus IMG export files (img)
  * Guys Image Processing Lab Format (gipl)
  * Tiff-Stack (tiff)

and the own defined formats (with export support)

  * XOCT (xml metadata with images (mostly png) in a zip file)
  * octbin (simple binary format for easy handling with matlab/octave)

Note that the most some file formats are reverse engineered and we have no guarantee that the information from the import are correct.

## License

This project is licensed under the LGPL3 License - see [license.txt](license.txt) file for details

## Build

for build instructions see the readme from the OCT-Marker project