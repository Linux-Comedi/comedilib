
include ../Config


CFLAGS += -I ../include -I .
LDFLAGS += -L../lib/ -lcomedi -lm


BINS=tut1 tut2
MBINS=inp inpn sv eeprom_dump info outp insn antialias ao_waveform \
	dio mmap ledclock receiver sigio select sender cmd poll

all: $(BINS) $(MBINS)

$(BINS): % : %.o
	$(CC) $(LDFLAGS) -o $@ $<

$(MBINS): % : %.o common.o
	$(CC) $(LDFLAGS) -o $@ common.o $<

clean:
	-rm -f *.o $(BINS) $(MBINS)
