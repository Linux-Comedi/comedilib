
## hack
FLEX_PATH=flex
BISON_PATH=bison

lib_LTLIBRARIES = libcomedi.la

noinst_HEADERS = calib_yacc.h comedi_errno.h libinternal.h

BUILT_SOURCES = calib_lex.c calib_yacc.c calib_yacc.h
CLEANFILES = calib_lex.c calib_yacc.c calib_yacc.h grammar.output

EXTRA_DIST = calib_lex.l calib_yacc.y version_script

nodist_libcomedi_la_SOURCES = calib_lex.c calib_yacc.c
libcomedi_la_SOURCES = \
	buffer.c calib.c cmd.c comedi.c data.c dio.c error.c \
	filler.c get.c ioctl.c range.c sv.c timed.c timer.c

libcomedi_la_CFLAGS = $(COMEDILIB_CFLAGS)
libcomedi_la_LDFLAGS = -Wl,--version-script=$(srcdir)/version_script

calib_yacc.c: $(srcdir)/calib_yacc.y
	$(BISON_PATH) -d -y -p calib_yy -o $@ $<

calib_lex.c: $(srcdir)/calib_lex.l
	$(FLEX_PATH) -Pcalib_yy -o$@ $<

