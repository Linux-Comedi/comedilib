
AC_INIT
AC_CANONICAL_TARGET([])

AC_CONFIG_AUX_DIR([.])
AS_VERSION(comedilib, COMEDILIB, 0, 7, 22)
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

#swig
AC_PATH_PROG(SWIG, swig, "no")
if test "$SWIG" == "no" ; then
	AC_MSG_WARN([swig not found, will not be able to build swig based bindings])
fi

#python
AC_ARG_ENABLE([python-binding],[  --disable-python-binding	Disable building of Python binding],
	[ENABLE_PYTHON=$enableval],[ENABLE_PYTHON="yes"])
if test "$ENABLE_PYTHON" == "yes" && test "$SWIG" != "no"; then
	AM_PATH_PYTHON
	AM_CHECK_PYTHON_HEADERS(HAVE_PYTHON=yes,[HAVE_PYTHON=no;AC_MSG_WARN([python headers not found, disabling python binding])])
else
	HAVE_PYTHON="no"
fi
AM_CONDITIONAL(HAVE_PYTHON, [test "$HAVE_PYTHON" == "yes" && test "$ENABLE_PYTHON" == "yes"])

AS_COMPILER_FLAG([-fno-strict-aliasing],[PYTHON_QUIET="$PYTHON_QUIET -fno-strict-aliasing"], true )
AS_COMPILER_FLAG([-Wno-unused-function],[PYTHON_QUIET="$PYTHON_QUIET -Wno-unused-function"], true )
AC_SUBST(PYTHON_QUIET)

# ruby
AC_ARG_ENABLE([ruby-binding], [  --disable-ruby-binding	Disable building of Ruby binding],
	[ENABLE_RUBY=$enableval], [ENABLE_RUBY="yes"])
if test "$ENABLE_RUBY" == "yes" && test "$SWIG" != "no"; then
	AC_PATH_PROG(RUBY, ruby, no)
	if test "$RUBY" != "no" ; then
		RUBY_INC_DIR=`$RUBY -e "require 'rbconfig'; c = ::Config::CONFIG; print c[['archdir']];"`
		AC_CHECK_HEADER([$RUBY_INC_DIR/ruby.h],[],[ENABLE_RUBY="no";AC_MSG_WARN([ruby.h not found, disabling Ruby binding])])
	else
		AC_MSG_WARN([ruby not found, disabling ruby binding])
		ENABLE_RUBY="no"
	fi
else
	ENABLE_RUBY="no"
fi

AM_CONDITIONAL(HAVE_RUBY, [test "$ENABLE_RUBY" == "yes"])

AC_ARG_VAR(RUBY_PREFIX,[path prefix for Ruby binding])
AC_ARG_VAR(RUBY_SO_DIR,[path for Ruby extensions])
if test "$RUBY_PREFIX" != "" ; then
	RUBY_CONFIG_OPTIONS="$RUBY_CONFIG_OPTIONS --prefix=\$(RUBY_PREFIX)"
fi
if test "$RUBY_SO_DIR" != "" ; then
	RUBY_CONFIG_OPTIONS="$RUBY_CONFIG_OPTIONS --so-dir=\$(RUBY_SO_DIR)"
fi
AC_SUBST(RUBY_CONFIG_OPTIONS)

# scxi
AC_ARG_ENABLE([scxi], [  --enable-scxi  Enable SCXI convenience library],
	[ENABLE_SCXI=$enableval], [ENABLE_SCXI="no"])
AM_CONDITIONAL(BUILD_SCXI, [test "$ENABLE_SCXI" == "yes"])

# docbook
AC_ARG_ENABLE([docbook],[  --disable-docbook-binding        Disable docbook],[ENABLE_DOCBOOK=$enableval],[ENABLE_DOCBOOK="yes"])

if test "$ENABLE_DOCBOOK" == "yes"; then
	AC_PATH_PROG(DOCBOOK2MAN, docbook2man, no)
	if test "$DOCBOOK2MAN" = "no" ; then
  		AC_MSG_WARN([docbook2man not found, will not be able to rebuild man pages])
	fi
else
	DOCBOOK2MAN="no"
fi
AM_CONDITIONAL(HAVE_DOCBOOK2MAN, [test "$DOCBOOK2MAN" != "no"])

if test "$ENABLE_DOCBOOK" == "yes"; then
	AC_PATH_PROG(DOCBOOK2PDF, docbook2pdf, no)
	if test "$DOCBOOK2PDF" = "no" ; then
		AC_MSG_WARN([docbook2pdf not found, will not be able to rebuild pdf documentation])
	fi
else
	DOCBOOK2PDF="no"
fi
AM_CONDITIONAL(HAVE_DOCBOOK2PDF, [test "$DOCBOOK2PDF" != "no"])


if test "$ENABLE_DOCBOOK" == "yes"; then
	AC_PATH_PROG(DOCBOOK2HTML, docbook2html, no)
	if test "$DOCBOOK2HTML" = "no" ; then
		AC_MSG_WARN([docbook2html not found, will not be able to rebuild html documentation])
	fi
else
	DOCBOOK2HTML="no"
fi
AM_CONDITIONAL(HAVE_DOCBOOK2HTML, [test "$DOCBOOK2HTML" != "no"])


pcmciadir="\${sysconfdir}/pcmcia"
AC_SUBST(pcmciadir)

#see: http://linux-hotplug.sourceforge.net/
#the hotplug expects the device dependent scripts here:
usbhotplugdir="\${sysconfdir}/hotplug/usb"
AC_SUBST(usbhotplugdir)

#firmware for the hotplug script
#see: http://linux-hotplug.sourceforge.net/
usbfirmwaredir="\${datadir}/usb"
AC_SUBST(usbfirmwaredir)

#documentaion goes here
doccomedilibdir="\${datadir}/doc/libcomedi0"
AC_SUBST(doccomedilibdir) 

AC_CONFIG_FILES(
Makefile
comedi_config/Makefile
etc/Makefile
etc/pcmcia/Makefile
etc/hotplug/Makefile
etc/hotplug/usb/Makefile
etc/hotplug/usb/usbdux/Makefile
etc/hotplug/usb/usbduxfast/Makefile
doc/Makefile
demo/Makefile
include/Makefile
lib/Makefile
man/Makefile
scxi/Makefile
swig/Makefile
swig/python/Makefile
swig/ruby/Makefile
testing/Makefile
comedilib.spec
comedilib.pc
)

AC_OUTPUT
