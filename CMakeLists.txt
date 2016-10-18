cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(liboctdata)
include(GenerateExportHeader)

set_property(GLOBAL PROPERTY CXX_STANDARD 11)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)


# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Build options
#
option(BUILD_WITH_SUPPORT_HE_E2E    "build support for nativ heidelberg spectralis format (E2E, SDB)" ON)
option(BUILD_WITH_SUPPORT_HE_VOL    "build support for heidelberg spectralis vol format" ON)
option(BUILD_WITH_SUPPORT_HE_XML    "build support for heidelberg spectralis xml export" ON)
option(BUILD_WITH_SUPPORT_DICOM     "build support for dicom format" OFF)
option(BUILD_WITH_SUPPORT_TIFFSTACK "build support for tiffstack" ON)
option(BUILD_WITH_SUPPORT_CI_RAW    "build support for cirrus raw files" ON)



# General build config
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_module/)

find_package(Boost 1.40 COMPONENTS filesystem system REQUIRED)
find_package(OpenCV REQUIRED)



if(BUILD_WITH_SUPPORT_DICOM)
	find_package(DCMTK REQUIRED)
	find_package(OpenJPEG REQUIRED)

endif()


find_package(LibE2E REQUIRED)
find_package(TIFF REQUIRED)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)


include_directories(SYSTEM ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/octdata/)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM ${TIFF_INCLUDE_DIRS})
include_directories(${LibE2E_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
	set(compiler_flags_debug "-Wall -Wextra -Wunreachable-code -Werror -Wconversion -Wold-style-cast")        ## -Weverything
	set(CMAKE_CXX_FLAGS "-std=c++11")
	set(CMAKE_CFLAGS "-fPIC")

	if(${WIN32})
		set(CMAKE_CXX_FLAGS "-mno-ms-bitfields -static -static-libstdc++ ${CMAKE_CXX_FLAGS}")
	endif()
endif()



file(GLOB sources_b "${CMAKE_CURRENT_SOURCE_DIR}/octdata/*.cpp")
file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/octdata/**/*.cpp")
file(GLOB sources_r "${CMAKE_CURRENT_SOURCE_DIR}/octdata/**/**/*.cpp")

list(APPEND sources ${sources_b} ${sources_r})


if(compiler_flags_debug)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${compiler_flags_debug}")
endif()
if(compiler_flags_release)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${compiler_flags_release}")
endif()
if(compiler_flags)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compiler_flags}")
endif()


add_library(octdata SHARED ${sources})


IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
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

GENERATE_EXPORT_HEADER(octdata           # generates the export header shared_EXPORTS.h automatically
	BASE_NAME octdata
	EXPORT_MACRO_NAME Octdata_EXPORTS
	EXPORT_FILE_NAME octdata_EXPORTS.h
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)


add_definitions(-DOCTDATA_EXPORT)

add_executable(liboctdata main.cpp)
target_link_libraries(liboctdata octdata)


