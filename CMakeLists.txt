cmake_minimum_required(VERSION 2.8)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME MME)
project(${PROJECT_NAME})

#set(${PROJECT_NAME}_VERSION "0.1.11")
file (STRINGS "VERSION" ${PROJECT_NAME}_VERSION)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
pkg_check_modules(LIBEVENT REQUIRED libevent)
pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.36.0)
pkg_check_modules(LIBCONFIG REQUIRED libconfig>=1.4.9)
add_definitions(${LIBCONFIG_CFLAGS})

find_package(SCTP REQUIRED)
find_package(MySQL REQUIRED)
find_package(OpenSSL REQUIRED)
#FindOpenSSL()

##Doxygen
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(UseDoxygen OPTIONAL)

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/Common
    /usr/include/mysql)

FILE(GLOB_RECURSE COMMON_SRC MME_SRCS "${PROJECT_SOURCE_DIR}/Common/*.c")

include_directories(
  ${COMMON_INCLUDES}
  ${GLIB2_INCLUDE_DIRS}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  mme/
  mme/S1
  mme/S6a
  mme/S6a/hmac
  mme/S6a/milenage
  mme/S1/NAS
  mme/S11
  mme/Controller)

FILE(GLOB_RECURSE MME_SRCS "mme/*.c")

add_definitions(-DMME_VERSION="${${PROJECT_NAME}_VERSION}")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("debug mode")
  add_definitions(-DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)


################################
# Normal Executables
################################
add_executable(${PROJECT_NAME} ${COMMON_SRC} ${MME_SRCS} mme.cfg)

target_link_libraries( ${PROJECT_NAME}
  gtp s1ap nas
  ${SCTP_LIBRARIES}
  ${LIBEVENT_LIBRARIES}
  ${LIBCONFIG_LIBRARIES}
  ${MYSQL_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${OPENSSL_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin COMPONENT binaries)
install(FILES mme.cfg DESTINATION /etc/cumucore/ COMPONENT config RENAME mme.cfg.template)
install(FILES MME.service DESTINATION /lib/systemd/system/ COMPONENT config)

################################
# GTP library : libgtp.a
################################

add_subdirectory(libgtp)

include_directories(libgtp/include)

################################
# S1AP library : libs1ap.a
################################

add_subdirectory(S1AP)

include_directories(S1AP/shared)

################################
# NAS library : libnas.a
################################

add_subdirectory(NAS)

include_directories(NAS/shared)


# Test
enable_testing()
add_subdirectory(test)

################################
# build a CPack driven installer package
################################
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vicent Ferrer Guasch <vicent.ferrerguasch@aalto.fi>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mobility Management Entity used in Evolved Packet Core.
 First version prepared for the phase 1 of LSA Testbed in Rome")
#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY"Mobility Management Entity used in Evolved Packet Core")
set(CPACK_DEBIAN_PACKAGE_SECTION Network)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_STRIP_FILES "1")
include (CPack)
