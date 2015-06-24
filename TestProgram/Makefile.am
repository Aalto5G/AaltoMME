## Process this file with automake to produce Makefile.in

TESTS = check_all

check_PROGRAMS = check_all

check_all_SOURCES = check_all.c check_libgtp.c check_rt_per.c check_libs1ap.c ../mme/storagesys.c ../Common/logmgr.c \
	check_nas.c \
	../S1AP/src/rt_per_bin.c ../S1AP/src/S1APlog.c

check_all_CFLAGS = @CHECK_CFLAGS@ \
	$(COVERAGE_CFLAGS)\
	$(GLIB_CFLAGS) $(GOBJECT_CFLAGS) \
	-I$(top_srcdir)/mme \
	-I$(top_srcdir)/S1AP/shared \
	-I$(top_srcdir)/libgtp \
	-I$(top_srcdir)/libgtp/include \
	-I$(top_srcdir)/mme/S1/NAS \
	-I$(top_srcdir)/mme/S6a/ \
	-I$(top_srcdir)/Common \
	-I$(top_srcdir)/S1AP/include/ \
	-I$(top_srcdir)/SDN/shared \
	-I$(top_srcdir)/NAS/shared 

check_all_LDADD = $(DBUS_TEST_RUNNER_LIBS) @CHECK_LIBS@ $(GLIB_LIBS) $(GOBJECT_LIBS)
check_all_LDFLAGS = $(COVERAGE_LDFLAGS) $(top_srcdir)/libgtp/libgtp.la -levent -lpcap $(top_srcdir)/NAS/src/libnas.la $(top_srcdir)/S1AP/libs1ap.la 
