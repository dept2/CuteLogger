# Try to find CuteLogger. Once done, this will define:
#
#  CuteLogger_INCLUDE_DIR - the NiftyMatch include directories
#  CuteLogger_LIBS - link these to use NiftyMatch
#

# to be kept in sync with CMakeLists.txt at top level
# allows defined suffix to be appended to all searched paths
SET(CuteLogger_PATH_SUFFIX cutelogger)

# Include dir
FIND_PATH(CuteLogger_INCLUDE_DIR
	NAMES Logger.h
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../include
	PATH_SUFFIXES ${CuteLogger_PATH_SUFFIX})

# And the modules of this library
FIND_LIBRARY(CuteLogger_LIB
	NAMES Logger
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../lib
	PATH_SUFFIXES ${CuteLogger_PATH_SUFFIX})

# Put them all into a var
SET(CuteLogger_LIBS
	${CuteLogger_LIB})

# handle the QUIETLY and REQUIRED arguments and set CuteLogger_FOUND
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	CuteLogger DEFAULT_MSG
	CuteLogger_LIBS CuteLogger_INCLUDE_DIR)