
AC_INIT
AC_CANONICAL_TARGET([])

AC_CONFIG_AUX_DIR([.])
AS_VERSION(comedilib, COMEDILIB, 0, 7, 21)
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
AM_PROG_LEX
AC_PROG_YACC

AC_HEADER_STDC([])

COMEDILIB_CFLAGS="$COMEDILIB_CFLAGS -I\$(top_srcdir)/include $COMEDILIB_DEBUG"
COMEDILIB_LIBS="$COMEDILIB_LIBS \$(top_builddir)/lib/libcomedi.la -lm"
AC_SUBST(COMEDILIB_CFLAGS)
AC_SUBST(COMEDILIB_LIBS)

AC_PATH_PROG(SWIG, swig, no)
if test "x$SWIG" == xno ; then
  AC_MSG_WARN([swig not found, will not be able to regenerate code for swig bindings])
fi
AM_CONDITIONAL(HAVE_SWIG, [test "x$SWIG" != xno])

AM_PATH_PYTHON
AM_CHECK_PYTHON_HEADERS(HAVE_PYTHON=yes,[HAVE_PYTHON=no;AC_MSG_WARN([python headers not found, disabling python binding])])
AM_CONDITIONAL(HAVE_PYTHON, [test "x$HAVE_PYTHON" = xyes])

AS_COMPILER_FLAG([-fno-strict-aliasing],[PYTHON_QUIET="$PYTHON_QUIET -fno-strict-aliasing"], true )
AS_COMPILER_FLAG([-Wno-unused-function],[PYTHON_QUIET="$PYTHON_QUIET -Wno-unused-function"], true )
AC_SUBST(PYTHON_QUIET)

AC_PATH_PROG(RUBY, ruby, no)
if test "x$RUBY" == xno ; then
  AC_MSG_WARN([ruby not found, disabling ruby binding])
fi
AM_CONDITIONAL(HAVE_RUBY, [test "x$RUBY" != xno])

AC_PATH_PROG(DOCBOOK2MAN, docbook2man, no)
if test x$DOCBOOK2MAN = xno ; then
  AC_MSG_WARN([docbook2man not found, will not be able to rebuild man pages])
fi
AM_CONDITIONAL(HAVE_DOCBOOK2MAN, [test "x$DOCBOOK2MAN" != xno])

AC_PATH_PROG(DOCBOOK2PDF, docbook2pdf, no)
if test x$DOCBOOK2PDF = xno ; then
  AC_MSG_WARN([docbook2pdf not found, will not be able to rebuild pdf documentation])
fi
AM_CONDITIONAL(HAVE_DOCBOOK2PDF, [test "x$DOCBOOK2PDF" != xno])

AC_PATH_PROG(DOCBOOK2HTML, docbook2html, no)
if test x$DOCBOOK2HTML = xno ; then
  AC_MSG_WARN([docbook2html not found, will not be able to rebuild html documentation])
fi
AM_CONDITIONAL(HAVE_DOCBOOK2HTML, [test "x$DOCBOOK2HTML" != xno])

pcmciadir="\${sysconfdir}/pcmcia"
AC_SUBST(pcmciadir)

#see: http://linux-hotplug.sourceforge.net/
#the hotplug expects the device dependent scripts here:
usbhotplugdir="/etc/hotplug/usb"
AC_SUBST(usbhotplugdir)

#firmware for the hotplug script
#see: http://linux-hotplug.sourceforge.net/
usbfirmwaredir="/usr/share/usb"
AC_SUBST(usbfirmwaredir)

AC_CONFIG_FILES(
Makefile
comedi_calibrate/Makefile
comedi_config/Makefile
etc/Makefile
etc/pcmcia/Makefile
etc/hotplug/Makefile
etc/hotplug/usb/Makefile
etc/hotplug/usb/usbdux/Makefile
doc/Makefile
demo/Makefile
include/Makefile
lib/Makefile
man/Makefile
swig/Makefile
swig/python/Makefile
swig/ruby/Makefile
testing/Makefile
comedilib.spec
)

AC_OUTPUT


