/*
   A little input demo for mode 4

   Mode 4 uses an external trigger to repeatedly trigger a
   scan of samples.  (This is different from mode 3, which
   triggers an individual sample.)  Thus, for each external
   trigger, n_chan samples are converted.

   If you have multiple external trigger lines, the
   particular trigger line is selected by trigval.

   The time between samples in a scan is selected
   by trigval1.  Conversion from seconds or Hz is done
   using the standard timer routines.

 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define N_SCANS		4
#define N_CHANS		4

int subdevice = 0;
int channels[N_CHANS] = { 0, 1, 2, 3 };
double freq = 1000;
int range = 0;
int aref = 3;
int external_trigger_number = 0;

#define N_SAMPLES	(N_CHANS*N_SCANS)

sampl_t data[4096];


int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_trig it;
	int err;
	int n,i,m;
	comedi_t *dev;
	double actual_freq;
	unsigned int chan[N_CHANS];

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	it.subdev = 0;
	it.mode = 2;
	it.flags = 0;
	it.n_chan = 1;
	it.chanlist = chan;
	it.data = data;
	it.n = 0; //N_SCANS;
	it.trigsrc = 0;
	it.trigvar = 10000;
	it.trigvar1 = 10000;

	/* pack the channel list */
	for(i=0;i<N_CHANS;i++){
		chan[i] = CR_PACK(channels[i], range, aref);
	}

	if ((err = comedi_trigger(dev, &it)) < 0) {
		perror("ioctl");
		exit(1);
	}

	m=0;
	while(1){
		if((n=read(comedi_fileno(dev),data,4096*sizeof(sampl_t)))<0){
			perror("read");
			exit(1);
		}
		if(n==0){
			perror("damn");
			exit(1);
		}
		n/=sizeof(sampl_t);
		m+=n;
		printf("read=%d total=%d data[0]=%d data[N-1]=%d\n",
			n,m,data[0],data[n-1]);
	}

	return 0;
}

