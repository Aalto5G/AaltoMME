
# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

FILE(GLOB_RECURSE MME_SRCS "*.c")
FILE(GLOB_RECURSE MME_INCLUDES "*.h")

include_directories(${MME_INCLUDES})
include_directories(${COMMON_INCLUDES})

################################
# Normal Libraries & Executables
################################
#add_library(project1_lib project1.cpp)
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere.

add_executable (mme ${MME_SRCS})
target_link_libraries(mme gtp s1ap nas)
