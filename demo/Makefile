


CFLAGS +=-I ../include -I . -O2
LDFLAGS=-L../lib/ -lcomedi -lm


BINS=mode4 mode3 mode2 ao_waveform tut2 cmd tut1 antialias
MBINS=inp inpn sv eeprom_dump info outp

all: $(patsubst %,_mbins_%,$(MBINS)) $(patsubst %,_bins_%,$(BINS))

$(patsubst %,_mbins_%,$(MBINS)) : main.o $(patsubst %,%.o,$(MBINS))
	$(CC) -o $(patsubst _mbins_%,%,$@) main.o $(patsubst _mbins_%,%.o,$@) $(LDFLAGS)

$(patsubst %,_bins_%,$(BINS)) : $(patsubst %,%.o,$(BINS))
	$(CC) -o $(patsubst _bins_%,%,$@) $(patsubst _bins_%,%.o,$@) $(LDFLAGS)

clean:
	-rm *.o $(BINS) $(MBINS)
