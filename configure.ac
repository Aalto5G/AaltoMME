dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([gtp],[1.0],[vicent.ferrerguasch@aalto.fi])
AC_PROG_CC

AC_CANONICAL_SYSTEM

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
dnl Initialize Libtool
LT_INIT
dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
AC_PROG_LIBTOOL

# Checks for libraries.
#AC_SEARCH_LIBS([config_error_file],[config],, AC_MSG_ERROR([libconfig not found.]))
AC_SEARCH_LIBS([event_base_loop],[event],, AC_MSG_ERROR([libevent not found.]))
AC_SEARCH_LIBS([sctp_sendmsg], [sctp],, AC_MSG_ERROR([libsctp not found.]))
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],, AC_MSG_WARN([Check test framework not found.]))
PKG_CHECK_MODULES([LIBCONFIG], [libconfig >= 1.3.2],, AC_MSG_ERROR([libconfig 1.3.2 or newer not found.]) )
PKG_CHECK_MODULES([GLIB], [glib-2.0 > 2.19.0],, AC_MSG_ERROR([glib 2.19.0 or newer not found.]) )
PKG_CHECK_MODULES([GOBJECT],[gobject-2.0])

# Line working on debian based, but failing on fedora. The mysqlclient will be checked looking for the header files
#AC_SEARCH_LIBS([mysql_init], [mysqlclient],, AC_MSG_ERROR([Please install mysqlclient library first.]))

# Checks for header files.
AC_CHECK_HEADERS([mysql/mysql.h],, AC_MSG_ERROR([Missing mysql Headers (mysql/mysql.h); intall the development packets and try again.]))
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h netinet/sctp.h ],,
 AC_MSG_ERROR([Missing Headers; intall the required development packets and try again.]))

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero gettimeofday inet_ntoa memmove memset socket strdup strerror strtoull])

dnl Initialize automake
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

WITH_MYSQL()

dnl this allows us specify individual liking flags for each target
AM_PROG_CC_C_O 

dnl remove optimization flags
changequote({,})
dnl CFLAGS=$(echo "$CFLAGS" | $SED -e 's/-O[0-9]*//g')
changequote([,])

dnl This mechanism allows one to enable debug compilations...
AC_ARG_ENABLE(debug,
[  --enable-debug     enable debugging and disable optimization],
[  if test "$enable_debug" = "yes"; then
      DEBUG_CFLAGS="-g3 -pg -fprofile-arcs -ftest-coverage -DDEBUG"
   else
      DEBUG_CFLAGS="-O2 -DNDEBUG"
   fi
]
)

AM_CONDITIONAL(ENABLE_DEBUG, test "$enable_debug" = "yes")
CFLAGS="$DEBUG_CFLAGS"


###########################
# gcov coverage reporting
###########################

AC_ARG_ENABLE(gcov,
	AS_HELP_STRING([--enable-gcov], [enable coverage testing with gcov]),
	[use_gcov=yes], [use_gcov=no])

AM_CONDITIONAL(HAVE_GCOV, test "x$use_gcov" = "xyes")

if test "x$use_gcov" = "xyes"; then
# we need gcc:
  if test "$GCC" != "yes"; then
    AC_MSG_ERROR([GCC is required for --enable-gcov])
  fi
fi

# Add the special gcc flags
#COVERAGE_CFLAGS="--coverage"
#COVERAGE_CXXFLAGS="--coverage"	
#COVERAGE_LDFLAGS="-lgcov"
COVERAGE_CFLAGS="-O0 -fprofile-arcs -ftest-coverage"
COVERAGE_CXXFLAGS="-O0 -fprofile-arcs -ftest-coverage"
COVERAGE_LDFLAGS="-lgcov"
#AC_SUBST(COVERAGE_CFLAGS)
#AC_SUBST(COVERAGE_CXXFLAGS)
#AC_SUBST(COVERAGE_LDFLAGS)

###########################
# Files #
##########################

AC_CONFIG_FILES(Makefile
                exampleProgram/Makefile
                libgtp/Makefile
                libgtp/include/Makefile
                TestProgram/Makefile
                S1AP/Makefile
                S1AP/src/Makefile
                S1AP/testApp/Makefile
                NAS/Makefile
                NAS/src/Makefile
                NAS/testApp/Makefile
                MMEcmd/Makefile
                mme/Makefile)
                
AC_OUTPUT
