


CFLAGS +=-I ../include -I . -O2 -Wall
LDFLAGS=-L../lib/ -lcomedi


BINS=comedi_calibrate

all: $(BINS)

$(patsubst %,_bins_%,$(BINS)) : dummy
	$(CC) -o $(patsubst _bins_%,%,$@) $(patsubst _bins_%,%.o,$@) $(LDFLAGS)

clean:
	-rm *.o $(BINS)
