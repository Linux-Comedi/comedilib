
include ../Config


CFLAGS += -I../include
LDFLAGS += -L../lib/ -lcomedi -lm


BINS=comedi_calibrate

all: $(BINS)

comedi_calibrate: comedi_calibrate.o
	$(CC) -o comedi_calibrate comedi_calibrate.o $(LDFLAGS)

clean:
	-rm -f *.o $(BINS)
