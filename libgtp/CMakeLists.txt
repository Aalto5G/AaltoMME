cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME gtp)
project(${PROJECT_NAME})

FILE(GLOB_RECURSE GTP_SRCS "*.c")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(gtp ${GTP_SRCS})
