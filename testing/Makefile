


CFLAGS +=-I ../include -I . -O2 -Wall
LDFLAGS=-L../lib/ -lcomedi


TARG=comedi_test
OBJS=main.o mode0_read.o insn_read.o info.o cmd_1.o

all: $(TARG)

$(TARG):	$(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LDFLAGS)

clean:
	-rm $(TARG) $(OBJS)

distclean: clean
