#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libnas.la

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -Wall -I../shared -I../include

#######################################
# Build information for each library

# Sources for libTest
libnas_la_SOURCES = NAS.c NASlog.c NASMessages.c StandardIeSchemas.c

# Linker options libTestProgram
libnas_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libnas_la_CPPFLAGS = -I$(top_srcdir)/NAS/include -I$(top_srcdir)/NAS/shared

