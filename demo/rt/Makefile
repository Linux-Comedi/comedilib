

# change this to your rtlinux include directory.
# I don't have RTLinux installed on the computer
# that I use to compile, so I need this:

LINUXDIR := /d/ds/cvs/rtl/linux22
RTINCDIR := /usr/include/rtlinux

CFLAGS = -O2 -Wall -D__KERNEL__ -DMODULE
CFLAGS += -I $(LINUXDIR)/include
CFLAGS += -I $(RTINCDIR) -O2 -Wall -D__RTL__
CFLAGS += -D__SMP__
CFLAGS += -DMODVERSIONS -include $(LINUXDIR)/include/linux/modversions.h

all: it.o ai.o

it.o:	it.c
	$(CC) $(CFLAGS) -o it.o -c it.c

ai.o:	ai.c
	$(CC) $(CFLAGS) -o ai.o -c ai.c

