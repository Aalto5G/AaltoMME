cmake_minimum_required(VERSION 2.8)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME s1ap)
project(${PROJECT_NAME})

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

FILE(GLOB_RECURSE S1AP_SRCS "src/*.c")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/shared)

################################
# Normal Libraries & Executables
################################
#add_library(project1_lib project1.cpp)
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere.

#target_link_libraries(gtp)

add_executable (testS1APApp testApp/S1APtest.c)
target_link_libraries(testS1APApp s1ap)

add_library(${PROJECT_NAME} ${S1AP_SRCS})

#install (TARGETS ${PROJECT_NAME} DESTINATION bin)
#install (FILES ${PROJECT_SOURCE_DIR}/include/*.h DESTINATION include)