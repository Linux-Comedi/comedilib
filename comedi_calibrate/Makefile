
include ../Config


CPPFLAGS += -I../include 
LDFLAGS += -L../lib/ -lcomedi -lm


BINS = comedi_calibrate
objs = comedi_calibrate.o ni.o cb.o cb64.o other.o save_cal.o

all: $(BINS)

comedi_calibrate: $(objs)
	$(CC) -o $@ $(objs) $(LDFLAGS)

clean:
	-rm -f *.o $(BINS)

#dependency stuff
%.d: %.c
	set -e; $(CC) -MM $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

include $(objs:.o=.d)
