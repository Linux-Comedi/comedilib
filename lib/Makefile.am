
lib_LTLIBRARIES = libcomedi.la

noinst_HEADERS = calib_lex.h calib_yacc.h libinternal.h

CLEANFILES = grammar.output

EXTRA_DIST = calib_lex.l calib_yacc.y version_script

BUILT_SOURCES = calib_lex.c calib_lex.h calib_yacc.c calib_yacc.h
MAINTAINERCLEANFILES = calib_lex.c calib_lex.h calib_yacc.c calib_yacc.h

libcomedi_la_SOURCES = \
	buffer.c calib.c cmd.c comedi.c data.c dio.c error.c \
	filler.c get.c ioctl.c range.c sv.c timed.c timer.c \
	calib_lex.c calib_yacc.c insn_config_wrappers.c

libcomedi_la_CFLAGS = $(COMEDILIB_CFLAGS) -D_REENTRANT -DLOCALSTATEDIR=\"\$(localstatedir)\"

libcomedi_la_LDFLAGS = -soname libcomedi \
        $(COMEDILIB_LT_LDFLAGS) \
        -Wl,--version-script=$(srcdir)/version_script

libcomedi_la_LIBADD = -lm

calib_yacc.c calib_yacc.h: calib_yacc.y
	$(YACC) -d -p _comedi_calib_yy -o calib_yacc.c $<

calib_lex.c calib_lex.h: calib_lex.l
	$(LEX) --header-file=calib_lex.h -P_comedi_calib_yy -o calib_lex.c $<
