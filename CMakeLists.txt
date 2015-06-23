cmake_minimum_required (VERSION 2.8)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME MME)
project(${PROJECT_NAME})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

###############################################################################
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)

###############################################################################
# The version number.
set (MME_VERSION_MAJOR 1)
set (MME_VERSION_MINOR 0)

##Doxygen
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(UseDoxygen OPTIONAL)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/Common)


include_directories ("${PROJECT_SOURCE_DIR}/libgtp/include")
add_subdirectory (libgtp)
include_directories ("${PROJECT_SOURCE_DIR}/S1AP/shared")
add_subdirectory (S1AP)
include_directories ("${PROJECT_SOURCE_DIR}/NAS/shared")
add_subdirectory (NAS)
include_directories ("${PROJECT_SOURCE_DIR}/SDN/shared")

add_subdirectory (mme)
include_directories ("${PROJECT_SOURCE_DIR}/SDN/shared")

#include_directories ("${PROJECT_SOURCE_DIR}/TestProgram")
#add_subdirectory (TestProgram)

# Unit tests
#enable_testing()
#add_test(NAME check_all COMMAND check_all)
