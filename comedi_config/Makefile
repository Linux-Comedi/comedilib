
include ../Config

CFLAGS += -I../include
LDFLAGS +=

OBJS=comedi_config.o

comedi_config: $(OBJS)
	$(CC) -o comedi_config $(OBJS) $(LDFLAGS)

clean:
	rm -f comedi_config *.o
