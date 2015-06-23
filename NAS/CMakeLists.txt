cmake_minimum_required(VERSION 2.8)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME nas)
project(${PROJECT_NAME})

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

FILE(GLOB_RECURSE NAS_SRCS "src/*.c")

include_directories(${PROJECT_SOURCE_DIR}/shared)
include_directories(${PROJECT_SOURCE_DIR}/include)

################################
# Normal Libraries & Executables
################################
#add_library(project1_lib project1.cpp)
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere.

add_executable (testNASApp testApp/NAStest.c)
target_link_libraries(testNASApp nas)

add_library(${PROJECT_NAME} ${NAS_SRCS})
