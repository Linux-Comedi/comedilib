


CFLAGS =-I ../include -I . -O2 -Wall -Wstrict-prototypes
LDFLAGS=-L../lib/ -lcomedi -lm


TARG=comedi_test
OBJS=main.o mode0_read.o insn_read.o info.o cmd_1.o insn_read_time.o \
	cmd_2.o mmap.o select.o cmd_3.o bufconfig.o lib.o inttrig.o

all: $(TARG)

$(TARG):	$(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LDFLAGS)

clean:
	-rm -f $(TARG) $(OBJS)

test: $(TARG)
	LD_LIBRARY_PATH=../lib COMEDILIB_LOGLEVEL=3 ./comedi_test -f /dev/comedi0 >out 2>&1
	-diff -u results/DAQCard-ai-16xe-50 out >xxx

distclean: clean
