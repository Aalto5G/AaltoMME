#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libs1ap.la


# additional include paths necessary to compile the C++ library
AM_CFLAGS = -Wall -I../shared -I../include

#######################################
# Build information for each library

# Sources for libTest
libs1ap_la_SOURCES = rt_per_bin.c S1AP.c CommonDataTypes.c Constants.c Containers.c S1APlog.c S1AP_PDU.c S1AP_IE.c S1AP_IEdec.c S1AP_IEenc.c 

# Linker options libTestProgram
libs1ap_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libs1ap_la_CPPFLAGS = -I$(top_srcdir)/S1AP/include -I$(top_srcdir)/S1AP/shared

