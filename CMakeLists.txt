cmake_minimum_required(VERSION 2.6)
project(liboctdata)
include(GenerateExportHeader)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_module/)

find_package(Boost 1.40 COMPONENTS filesystem system REQUIRED)
find_package(OpenCV REQUIRED)
find_package(DCMTK QUIET)
find_package(LibE2E REQUIRED)
find_package(OpenJPEG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/octdata/)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LibE2E_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
	set(compiler_flags_debug "-Wall -Wextra -Wunreachable-code -Werror -Wconversion")        ## -Weverything
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


if(DEFINED DCMTK_FOUND)
	add_definitions(-DUSE_DCMTK)
	target_link_libraries(octdata ${DCMTK_LIBRARIES})
endif()
target_link_libraries(octdata ${LibE2E_LIBRARIES} ${OPENJPEG_LIBRARIES} ${Boost_LIBRARIES})

GENERATE_EXPORT_HEADER(octdata           # generates the export header shared_EXPORTS.h automatically
    BASE_NAME octdata
    EXPORT_MACRO_NAME octdata_EXPORTS
    EXPORT_FILE_NAME octdata_EXPORTS.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

add_executable(liboctdata main.cpp)
target_link_libraries(liboctdata octdata ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})


