
lib_LTLIBRARIES = libcomedi.la

noinst_HEADERS = calib_yacc.h comedi_errno.h libinternal.h

libcomedi_la_SOURCES = \
	buffer.c calib.c calib_lex.c calib_yacc.c cmd.c comedi.c \
	data.c dio.c error.c filler.c get.c ioctl.c range.c sv.c \
	timed.c timer.c

libcomedi_la_CFLAGS = $(COMEDILIB_CFLAGS)
libcomedi_la_LDFLAGS = -Wl,--version-script=$(srcdir)/version_script

EXTRA_DIST = version_script

