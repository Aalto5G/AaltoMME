## Process this file with automake to produce Makefile.in

TESTS = check_all
check_PROGRAMS = check_all
check_all_SOURCES = check_all.c check_libgtp.c check_rt_per.c check_libs1ap.c $(top_srcdir)/mme/storagesys.c $(top_srcdir)/Common/logmgr.c \
	check_nas.c \
	$(top_builddir)/S1AP/src/rt_per_bin.c $(top_builddir)/S1AP/src/S1APlog.c
check_all_CFLAGS = @CHECK_CFLAGS@ \
	$(COVERAGE_CFLAGS)\
	-I$(top_srcdir)/mme \
	-I$(top_srcdir)/S1AP/shared \
	-I$(top_builddir)/libgtp \
	-I$(top_srcdir)/libgtp/include \
	-I$(top_srcdir)/mme/S1/NAS \
	-I$(top_srcdir)/mme/S6a/ \
	-I$(top_srcdir)/Common \
	-I$(top_srcdir)/S1AP/include/ \
	-I$(top_srcdir)/SDN/shared \
	-I$(top_srcdir)/NAS/shared 
	
check_all_LDADD = $(DBUS_TEST_RUNNER_LIBS) $(top_builddir)/libgtp/libgtp.la -levent -lpcap $(top_builddir)/NAS/src/libnas.la $(top_builddir)/S1AP/src/libs1ap.la @CHECK_LIBS@
check_all_LDFLAGS = $(COVERAGE_LDFLAGS)
