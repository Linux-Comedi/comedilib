


CFLAGS += -I ../include -I . -O2 -Wall -Wstrict-prototypes
LDFLAGS = -L../lib/ -lcomedi -lm


BINS=cmd tut1 tut2
MBINS=inp inpn sv eeprom_dump info outp insn antialias ao_waveform dio mmap

all: $(patsubst %,_mbins_%,$(MBINS)) $(patsubst %,_bins_%,$(BINS))

$(patsubst %,_mbins_%,$(MBINS)) : main.o $(patsubst %,%.o,$(MBINS))
	$(CC) -o $(patsubst _mbins_%,%,$@) main.o $(patsubst _mbins_%,%.o,$@) $(LDFLAGS)

$(patsubst %,_bins_%,$(BINS)) : $(patsubst %,%.o,$(BINS))
	$(CC) -o $(patsubst _bins_%,%,$@) $(patsubst _bins_%,%.o,$@) $(LDFLAGS)

clean:
	-rm -f *.o $(BINS) $(MBINS)
