
include ../Config
include ../version

CPPFLAGS += -I../include -I.
CFLAGS += -fPIC -D_REENTRANT

OBJS=comedi.o timer.o sv.o range.o ioctl.o filler.o timed.o error.o \
	dio.o data.o get.o cmd.o buffer.o calib.o calib_lex.o calib_yacc.o

SONAME=libcomedi$(SONAME_SUFFIX).so.0

libcomedi.a: $(OBJS) version_script
	#$(CC) -shared -Wl,-soname,libcomedi.so,-T,version_script -o libcomedi.so.${VERSION_CODE} $(OBJS) -lm
	$(CC) -shared -Wl,-soname,$(SONAME) -Wl,--version-script,version_script -o libcomedi.so.${version} $(OBJS) -lm
	$(AR) rs libcomedi.a $(OBJS)
	ln -sf libcomedi.so.${version} libcomedi.so
	ln -sf libcomedi.so.${version} libcomedi.so.0

clean:
	rm -f libcomedi.a libcomedi.so* *.o calib_lex.c calib_yacc.c calib_yacc.h

calib_lex.c:  calib_lex.l calib_yacc.h
	flex -Pcalib_yy -o$@ $<

calib_yacc.c + calib_yacc.h: calib_yacc.y
	bison -d -y -p calib_yy -o calib_yacc.c calib_yacc.y

#dependency stuff
%.d: %.c
	set -e; $(CC) -MM $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

include $(OBJS:.o=.d)
