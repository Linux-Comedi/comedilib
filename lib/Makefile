
include ../Config
include ../version

PERL_LDFLAGS := -lperl $(shell perl -MConfig -e 'print $$Config{perllibs}')
PERL_INC := $(shell perl -MConfig -e 'print $$Config{archlib}')/CORE

CFLAGS += -fPIC -I../include -I. -I$(PERL_INC)

OBJS=comedi.o timer.o sv.o range.o ioctl.o filler.o timed.o error.o \
	dio.o data.o get.o cmd.o buffer.o calib.o

SONAME=libcomedi$(SONAME_SUFFIX).so.0

libcomedi.a: $(OBJS) version_script
	#$(CC) -shared -Wl,-soname,libcomedi.so,-T,version_script -o libcomedi.so.${VERSION_CODE} $(OBJS) -lm
	$(CC) -shared -Wl,-soname,$(SONAME) -Wl,--version-script,version_script -o libcomedi.so.${version} $(OBJS) -lm $(PERL_LDFLAGS)
	$(AR) rs libcomedi.a $(OBJS)
	ln -sf libcomedi.so.${version} libcomedi.so
	ln -sf libcomedi.so.${version} libcomedi.so.0

clean:
	rm -f libcomedi.a libcomedi.so* *.o

