


CFLAGS +=-I ../include -I . -O2 -Wall
LDFLAGS=-L../lib/ -lcomedi -lm


BINS=comedi_calibrate

all: $(BINS)

comedi_calibrate: comedi_calibrate.o
	$(CC) -o comedi_calibrate comedi_calibrate.o $(LDFLAGS)

clean:
	-rm *.o $(BINS)
