
include ../Config


CFLAGS += -I../include
LDFLAGS += -L../lib/ -lcomedi -lm


BINS = comedi_calibrate
objs = comedi_calibrate.o ni.o cb.o

all: $(BINS)

comedi_calibrate: $(objs)
	$(CC) -o $@ $(objs) $(LDFLAGS)

clean:
	-rm -f *.o $(BINS)
