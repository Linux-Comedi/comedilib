
AC_INIT
AC_CANONICAL_TARGET([])

AS_VERSION(comedilib, COMEDILIB, 0, 7, 19, 1)
#COMEDILIB_DEBUG="-Wall -Werror"
COMEDILIB_DEBUG="-Wall"
AM_INIT_AUTOMAKE($PACKAGE,$VERSION)
AS_LIBTOOL(COMEDILIB, 0, 7, 18, yes)

AM_CONFIG_HEADER(config.h)

ACLOCAL="$ACLOCAL -I m4"

AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_CC_STDC
AC_ISC_POSIX

AC_HEADER_STDC([])

COMEDILIB_CFLAGS="$COMEDILIB_CFLAGS -I\$(top_srcdir)/include $COMEDILIB_DEBUG"
COMEDILIB_LIBS="$COMEDILIB_LIBS \$(top_builddir)/lib/libcomedi.la -lm"
AC_SUBST(COMEDILIB_CFLAGS)
AC_SUBST(COMEDILIB_LIBS)

AM_PATH_PYTHON
AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])


AC_CONFIG_FILES(
Makefile
comedi_calibrate/Makefile
comedi_config/Makefile
doc/Makefile
demo/Makefile
include/Makefile
lib/Makefile
man/Makefile
python/Makefile
testing/Makefile
comedilib.spec
)

AC_OUTPUT


