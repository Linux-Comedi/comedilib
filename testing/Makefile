
include ../Config

CFLAGS += -I../include -I.
LDFLAGS += -L../lib/ -lcomedi -lm


TARG=comedi_test
OBJS=main.o mode0_read.o insn_read.o info.o cmd_1.o insn_read_time.o \
	cmd_2.o mmap.o select.o cmd_3.o bufconfig.o lib.o inttrig.o

all: $(TARG)

$(TARG):	$(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LDFLAGS)

clean:
	-rm -f $(TARG) $(OBJS) test.out test.diff

test: $(TARG)
	LD_LIBRARY_PATH=../lib COMEDILIB_LOGLEVEL=3 ./comedi_test -f /dev/comedi0 >test.out 2>&1
	devicename=`grep 'I: device name:' test.out | sed 's/I: device name: //'`
	-diff -u results/`grep 'I: device name:' test.out | sed 's/I: device name: //'` test.out >test.diff

distclean: clean
