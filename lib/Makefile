
include ../Config
include ../version

CFLAGS += -fPIC -I../include -I.

OBJS=comedi.o timer.o sv.o range.o ioctl.o filler.o timed.o error.o \
	dio.o data.o get.o cmd.o buffer.o

SONAME=libcomedi$(SONAME_SUFFIX).so.0

libcomedi.a: $(OBJS)
	#$(CC) -shared -Wl,-soname,libcomedi.so,-T,version_script -o libcomedi.so.${VERSION_CODE} $(OBJS) -lm
	$(CC) -shared -Wl,-soname,$(SONAME) -o libcomedi.so.${version} $(OBJS) -lm
	$(AR) rs libcomedi.a $(OBJS)
	ln -sf libcomedi.so.${version} libcomedi.so
	ln -sf libcomedi.so.${version} libcomedi.so.0

clean:
	rm -f libcomedi.a libcomedi.so* *.o

