/*
   A little input demo for mode 2

   Mode 2 uses two different timers to convert samples.
   The primary timer determines the time between scans,
   and the secondary timer determines the time between
   samples in a scan.

   The time between scans is in trigval; the time
   between samples is selected by trigval1.  Conversion
   from seconds or Hz is done using the standard timer
   routines.

 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define N_SCANS		10
#define N_CHANS		16

int subdevice = 0;
int chan=0;
int range = 0;
int aref = AREF_GROUND;
double freq = 1000;

#define N_SAMPLES	1000

double data[N_SAMPLES];


int main(int argc, char *argv[])
{
	char *fn = NULL;
	int i;
	comedi_t *dev;

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

#if 0
	for(i=0;i<10;i++){
		range=comedi_find_range(dev,subdevice,chan,0,-i,i);
		printf("%d\n",range);
	}
#endif
	comedi_timed_1chan(dev,subdevice,chan,range,aref,freq,N_SAMPLES,data);

	for(i=0;i<N_SAMPLES;i++){
		printf("%g\n",data[i]);
	}

	return 0;
}

