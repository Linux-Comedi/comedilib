

# change this to your rtlinux include directory.
# I don't have RTLinux installed on the computer
# that I use to compile, so I need this:

RTINCDIR = -I/b/ds/stuff/rtlinux/r-9J/rtl/include

CFLAGS = -I../../include $(RTINCDIR) -O2 -Wall -D__RT__ -D__KERNEL__ -DMODULE

all: it.o ai.o

it.o:	it.c
	$(CC) $(CFLAGS) -o it.o -c it.c

ai.o:	ai.c
	$(CC) $(CFLAGS) -o ai.o -c ai.c

