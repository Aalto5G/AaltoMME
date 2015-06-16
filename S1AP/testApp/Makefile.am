#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
#bin_PROGRAMS=exampleProgram

# Because a.out is only a sample program we don't want it to be installed.
# The 'noinst_' prefix indicates that the following targets are not to be
# installed.
noinst_PROGRAMS=S1APtest

AM_CFLAGS = -Wall -I$(top_srcdir)/S1AP/shared
#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources for the a.out 
S1APtest_SOURCES = S1APtest.c

# Linker options for a.out
S1APtest_LDFLAGS = $(top_builddir)/S1AP/src/libs1ap.la

# Compiler options for a.out
#S1APtest__CPPFLAGS = -Wall -I$(top_srcdir)/S1AP/shared

