


CFLAGS += -I ../include -I . -O2 -Wall -Wstrict-prototypes
#CFLAGS += -I ../include -I . -Wall -Wstrict-prototypes -g
LDFLAGS = -L../lib/ -lcomedi -lm


BINS=tut1 tut2
MBINS=inp inpn sv eeprom_dump info outp insn antialias ao_waveform \
	dio mmap ledclock receiver sigio select sender cmd

all: $(BINS) $(MBINS)

$(BINS): % : %.o
	$(CC) $(LDFLAGS) -o $@ $<

$(MBINS): % : %.o main.o
	$(CC) $(LDFLAGS) -o $@ main.o $<

clean:
	-rm -f *.o $(BINS) $(MBINS)
