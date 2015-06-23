lib_LTLIBRARIES = libs1ap.la

AM_CFLAGS = -Ishared -Iinclude

libs1ap_la_SOURCES = src/rt_per_bin.c src/S1AP.c src/CommonDataTypes.c src/Constants.c src/Containers.c src/S1APlog.c src/S1AP_PDU.c src/S1AP_IE.c src/S1AP_IEdec.c src/S1AP_IEenc.c 

libs1ap_la_CPPFLAGS = -I$(top_srcdir)/S1AP/include -I$(top_srcdir)/S1AP/shared


noinst_PROGRAMS=S1APtest
S1APtest_SOURCES = testApp/S1APtest.c
S1APtest_LDFLAGS = $(top_builddir)/S1AP/libs1ap.la
