#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libgtp.la

#######################################
# Build information for each library

# Sources for libTest
libgtp_la_SOURCES = gtp.c gtpie.c

# Linker options libTestProgram
libgtp_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libgtp_la_CPPFLAGS = -I$(top_srcdir)/libgtp/include

