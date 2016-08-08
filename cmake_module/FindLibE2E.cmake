# - Try to find LibE2E
# Once done this will define
#  LibE2E_FOUND - System has LibE2E
#  LibE2E_INCLUDE_DIRS - The LibE2E include directories
#  LibE2E_LIBRARIES - The libraries needed to use LibE2E
#  LibE2E_DEFINITIONS - Compiler switches required for using LibE2E

find_package(PkgConfig)
# pkg_check_modules(PC_LIBXML QUIET libxml-2.0)
# set(LibE2E_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(LibE2E_INCLUDE_DIR NAMES E2E PATHS ../
          # HINTS ${PC_LIBXML_INCLUDEDIR} ${PC_LIBXML_INCLUDE_DIRS}
          PATH_SUFFIXES LibE2E
          )

message(${LibE2E_INCLUDE_DIR})

find_library(LibE2E_LIBRARY NAMES libe2e.a
			PATHS ${LibE2E_INCLUDE_DIR}
			PATH_SUFFIXES build release
             # HINTS ${PC_LIBXML_LIBDIR} ${PC_LIBXML_LIBRARY_DIRS}
             )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LibE2E_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibE2E DEFAULT_MSG
                                  LibE2E_LIBRARY LibE2E_INCLUDE_DIR)

mark_as_advanced(LibE2E_INCLUDE_DIR LibE2E_LIBRARY )

set(LibE2E_LIBRARIES ${LibE2E_LIBRARY} )
set(LibE2E_INCLUDE_DIRS ${LibE2E_INCLUDE_DIR} )
