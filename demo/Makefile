
include ../Config


CFLAGS += -I ../include -I .
LIBS += ../lib/libcomedi.a -lm


BINS=tut1 tut2 # sv
MBINS=inp inpn eeprom_dump info outp insn antialias ao_waveform \
	dio mmap ledclock receiver sigio select sender cmd poll

all: $(BINS) $(MBINS)

$(BINS): % : %.o
	$(CC) -o $@ $< $(LIBS)

$(MBINS): % : %.o common.o
	$(CC) -o $@ common.o $< $(LIBS)

clean:
	-rm -f *.o $(BINS) $(MBINS)
