cmake_minimum_required(VERSION 2.8)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME gtp)
project(${PROJECT_NAME})

set(${PROJECT_NAME}_VERSION_MAJOR "0")
set(${PROJECT_NAME}_VERSION_MINOR "1")
set(${PROJECT_NAME}_VERSION_PATCH "0")

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

FILE(GLOB_RECURSE GTP_SRCS "*.c")
add_definitions(-DVERSION="${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
include_directories(${PROJECT_SOURCE_DIR}/include)

################################
# Normal Libraries & Executables
################################
#add_library(project1_lib project1.cpp)
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere.

#target_link_libraries(gtp)

add_library(${PROJECT_NAME} ${GTP_SRCS})

#install (TARGETS ${PROJECT_NAME} DESTINATION bin)
#install (FILES ${PROJECT_SOURCE_DIR}/include/*.h DESTINATION include)