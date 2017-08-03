cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(liboctdata)
include(GenerateExportHeader)

set_property(GLOBAL PROPERTY CXX_STANDARD 11)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Build options
#
option(BUILD_WITH_SUPPORT_HE_E2E    "build support for nativ heidelberg spectralis format (E2E, SDB)" ON)
option(BUILD_WITH_SUPPORT_HE_VOL    "build support for heidelberg spectralis vol format" ON)
option(BUILD_WITH_SUPPORT_HE_XML    "build support for heidelberg spectralis xml export" ON)
option(BUILD_WITH_SUPPORT_DICOM     "build support for dicom format" OFF)
option(BUILD_WITH_SUPPORT_TIFFSTACK "build support for tiffstack" ON)
option(BUILD_WITH_SUPPORT_CVBIN     "build support for cvbin import" ON)
option(BUILD_WITH_SUPPORT_CI_RAW    "build support for cirrus raw files" ON)
option(BUILD_WITH_SUPPORT_OCT_FILE  "build support for oct format" ON)
option(BUILD_WITH_SUPPORT_TOPCON    "build support for topcon format" ON)
option(BUILD_WITH_SUPPORT_GIPL      "build support for gipl format" ON)


# General build config
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_module/)

find_package(CppFramework REQUIRED)
include_directories(${CppFramework_INCLUDE_DIRS})


if(${WIN32})
	# set(Boost_DEBUG ON)
	# set(Boost_LIB_PREFIX "libboost")
else()
	ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
endif()



find_package(Boost 1.40 COMPONENTS filesystem system locale log REQUIRED)
find_package(OpenCV REQUIRED)
string(TIMESTAMP CMAKE_CONFIGURE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)


include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)


if(BUILD_WITH_SUPPORT_CVBIN)

	list(APPEND import_srcs import/cvbin)
	add_definitions(-DCVBIN_SUPPORT)
endif()

if(BUILD_WITH_SUPPORT_DICOM)
	find_package(DCMTK REQUIRED)
	find_package(OpenJPEG)
	list(APPEND import_srcs import/dicom)
	
	add_definitions(-DDICOM_SUPPORT)
		
	if(DEFINED OpenJPEG_FOUND)
		add_definitions(-DDICOM_JPEG2K_SUPPORT)
	endif()
endif()


if(BUILD_WITH_SUPPORT_CI_RAW)
	list(APPEND import_srcs import/cirrus_raw)
	add_definitions(-DCIRRUS_RAW_SUPPORT)
endif()


if(BUILD_WITH_SUPPORT_HE_E2E)
	find_package(LibE2E REQUIRED)
	list(APPEND import_srcs import/he_e2e)
	
	add_definitions(-DHE_E2E_SUPPORT)
endif()


if(BUILD_WITH_SUPPORT_HE_VOL)
	list(APPEND import_srcs import/he_vol)
	
	add_definitions(-DHE_VOL_SUPPORT)
endif()

if(BUILD_WITH_SUPPORT_OCT_FILE)
	list(APPEND import_srcs import/oct)

	add_definitions(-DOCT_FILE_SUPPORT)
endif()

if(BUILD_WITH_SUPPORT_TOPCON)
	find_package(OpenJPEG)
	if(DEFINED OpenJPEG_FOUND)
		add_definitions(-DDICOM_JPEG2K_SUPPORT)
		list(APPEND import_srcs import/topcon)
		add_definitions(-DTOPCON_FILE_SUPPORT)
		include_directories(SYSTEM ${OPENJPEG_INCLUDE_DIRS})
		message(${OPENJPEG_INCLUDE_DIRS})
	endif()
endif()

if(BUILD_WITH_SUPPORT_GIPL)
	list(APPEND import_srcs import/gipl)

	add_definitions(-DGIPL_SUPPORT)
endif()

if(BUILD_WITH_SUPPORT_HE_XML)
	list(APPEND import_srcs import/he_xml)
	
	add_definitions(-DHE_XML_SUPPORT)
endif()


if(BUILD_WITH_SUPPORT_TIFFSTACK)
	find_package(TIFF REQUIRED)
	list(APPEND import_srcs import/tiffstack)
	
	add_definitions(-DTIFFSTACK_SUPPORT)
endif()
	
	
	

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)


include_directories(SYSTEM ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/octdata/)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM ${TIFF_INCLUDE_DIRS})
include_directories(${LibE2E_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wunreachable-code -Werror -Wconversion -Wold-style-cast")        ## -Weverything
	set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -fPIC")

	if(${WIN32})
		set(CMAKE_CXX_FLAGS "-mno-ms-bitfields -static -static-libstdc++ ${CMAKE_CXX_FLAGS}")
	endif()
endif()


file(GLOB liboctdata_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/octdata/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/octdata/**/*.cpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/octdata/buildconstants.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/buildconstants.cpp" @ONLY)
list(APPEND liboctdata_SRCS "${CMAKE_CURRENT_BINARY_DIR}/buildconstants.cpp")


set(srcs_directories datastruct ${import_srcs} "export/cvbin")
foreach(loop_var ${srcs_directories})
	file(GLOB sources_base "${CMAKE_CURRENT_SOURCE_DIR}/octdata/${loop_var}/*.cpp")
	list(APPEND liboctdata_SRCS ${sources} ${sources_base})
endforeach(loop_var)


# hack for qtcreator (show header files)
FILE(GLOB_RECURSE includefiles "octdata/*.h")
add_custom_target(headers SOURCES ${includefiles})


add_library(octdata SHARED ${liboctdata_SRCS})


IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  SET_TARGET_PROPERTIES(octdata PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)


if(DEFINED DCMTK_FOUND)
	add_definitions(-DUSE_DCMTK)
	target_link_libraries(octdata ${DCMTK_LIBRARIES})
endif()
# target_link_libraries(octdata ${LibE2E_LIBRARIES} ${OPENJPEG_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(octdata ${LibE2E_LIBRARIES} ${OPENJPEG_LIBRARIES} ${TIFF_LIBRARIES} ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(octdata ${CppFramework_LIBRARIES})

GENERATE_EXPORT_HEADER(octdata           # generates the export header shared_EXPORTS.h automatically
	BASE_NAME octdata
	EXPORT_MACRO_NAME Octdata_EXPORTS
	EXPORT_FILE_NAME octdata_EXPORTS.h
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)


add_definitions(-DOCTDATA_EXPORT)

add_executable(liboctdata_test main.cpp)
target_link_libraries(liboctdata_test octdata)


