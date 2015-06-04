#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
#bin_PROGRAMS=exampleProgram

# Because a.out is only a sample program we don't want it to be installed.
# The 'noinst_' prefix indicates that the following targets are not to be
# installed.
noinst_PROGRAMS=exampleProgram eping loadtest eNBemulator hmac_test loadWithAttach

#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources for the a.out
eping_SORCES = eping.c ../mme/storagesys.c ../Common/logmgr.c
loadtest_SOURCES = loadtest.c ../mme/storagesys.c ../Common/logmgr.c
exampleProgram_SOURCES= exampleProgram.c ../mme/storagesys.c ../Common/logmgr.c
eNBemulator_SOURCES = eNBemulator.c ../Common/logmgr.c
hmac_test_SOURCES = hmac_test.c ../mme/S6a/hmac/sha2.c ../mme/S6a/hmac/hmac_sha2.c
loadWithAttach_SOURCES = loadWithAttach.c


# Linker options for a.out
eping_LDFLAGS = $(top_srcdir)/libgtp/libgtp.la
exampleProgram_LDFLAGS = $(top_srcdir)/libgtp/libgtp.la
# eNBemulator_LDFLAGS = $(top_srcdir)/S1AP/src/libs1ap.la $(top_srcdir)/NAS/src/libnas.la
loadWithAttach_LDFLAGS = $(top_srcdir)/S1AP/src/libs1ap.la $(top_srcdir)/NAS/src/libnas.la `mysql_config --libs_r`
#
exampleProgram_LDADD = -levent
eping_LDADD = -levent
eNBemulator_LDFLAGS = -levent -lsctp
loadWithAttach_LDADD = -levent -lsctp
#

# -ls1ap -lnas



# Compiler options for a.out
eping_CPPFLAGS = -Wall -I$(top_srcdir)/libgtp/include \
			 -I$(top_srcdir)/mme \
			 -I$(top_srcdir)/Common \
			 -I$(top_srcdir)/mme/S1 \
			 -I$(top_srcdir)/S1AP/shared \
			 -I$(top_srcdir)/mme/S1/NAS \
			 -I$(top_srcdir)/NAS/shared \
			 -I$(top_srcdir)/SDN/shared \
			 -I$(top_srcdir)/mme/S6a/ \
			 -I$(top_srcdir)/mme/S11
loadtest_CPPFLAGS = -Wall -I$(top_srcdir)/libgtp/include \
			 -I$(top_srcdir)/mme \
			 -I$(top_srcdir)/Common \
			 -I$(top_srcdir)/mme/S1 \
			 -I$(top_srcdir)/S1AP/shared \
			 -I$(top_srcdir)/mme/S1/NAS \
			 -I$(top_srcdir)/NAS/shared \
			 -I$(top_srcdir)/SDN/shared \
			 -I$(top_srcdir)/mme/S6a/ \
			 -I$(top_srcdir)/mme/S11


exampleProgram_CPPFLAGS = -Wall -I$(top_srcdir)/libgtp/include \
			 -I$(top_srcdir)/mme \
			 -I$(top_srcdir)/Common \
			 -I$(top_srcdir)/mme/S1 \
			 -I$(top_srcdir)/S1AP/shared \
			 -I$(top_srcdir)/mme/S6a/ \
			 -I$(top_srcdir)/mme/S1/NAS \
			 -I$(top_srcdir)/NAS/shared \
			 -I$(top_srcdir)/SDN/shared \
			 -I$(top_srcdir)/mme/S11

eNBemulator_CPPFLAGS = -Wall -I$(top_srcdir)/libgtp/include \
			 -I$(top_srcdir)/Common \
			 -I$(top_srcdir)/S1AP/shared

loadWithAttach_CPPFLAGS = -Wall -I$(top_srcdir)/libgtp/include \
			 `mysql_config --cflags` \
			 -I$(top_srcdir)/NAS/shared \
			 -I$(top_srcdir)/SDN/shared \
			 -I$(top_srcdir)/S1AP/shared

#			 -I$(top_srcdir)/mme/S1 \
#			 -I$(top_srcdir)/mme \
#			 -I$(top_srcdir)/mme/S1/NAS \
#			 -I$(top_srcdir)/NAS/shared \
#			 -I$(top_srcdir)/mme/S11


hmac_test_CPPFLAGS = -Wall -I$(top_srcdir)/mme/S6a/hmac
