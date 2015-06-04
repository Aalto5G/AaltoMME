#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
#bin_PROGRAMS=exampleProgram
bin_PROGRAMS=MMEcmd

#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources
AM_CXXFLAGS = $(INTI_CFLAGS)
MMEcmd_SOURCES = MMEcmd.c ../Common/logmgr.c

# Linker options
MMEcmd_LDADD = $(INTI_LIBS) -levent

# Compiler options 
MMEcmd_CFLAGS = -I$(top_srcdir)/libgtp/include -I$(top_srcdir)/Common -I$(top_srcdir)/mme
