
CFLAGS=-fPIC -Wall -O2 -I../include -I.

OBJS=comedi.o timer.o sv.o range.o ioctl.o filler.o timed.o error.o \
	dio.o data.o get.o command.o

libcomedi.a: $(OBJS)
	#gcc -shared -Wl,-soname,libcomedi.so,-T,version_script -o libcomedi.so.${VERSION_CODE} $(OBJS) -lm
	gcc -shared -Wl,-soname,libcomedi.so.0 -o libcomedi.so.${version} $(OBJS) -lm
	ar rs libcomedi.a $(OBJS)
	ln -sf libcomedi.so.${version} libcomedi.so
	ln -sf libcomedi.so.${version} libcomedi.so.0

clean:
	rm -f libcomedi.a libcomedi.so* *.o


