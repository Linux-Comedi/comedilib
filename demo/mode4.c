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

sampl_t data[N_SAMPLES];


int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_trig it;
	int err;
	int n,i;
	comedi_t *dev;
	double actual_freq;
	unsigned int chan[N_CHANS];

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	it.subdev = 0;
	it.mode = 4;
	it.flags = 0;
	it.n_chan = N_CHANS;
	it.chanlist = chan;
	it.data = data;
	it.n = N_SCANS;
	it.trigsrc = 0;

	/* external trigger number */
	it.trigvar = external_trigger_number;

	/* convert the frequency into a timer value */
	comedi_get_timer(dev,subdevice,freq,&it.trigvar1,&actual_freq);
	printf("actual frequency=%g timer value=%d\n",actual_freq,it.trigvar1);

	/* pack the channel list */
	for(i=0;i<N_CHANS;i++){
		chan[i] = CR_PACK(channels[i], range, aref);
	}

	if ((err = comedi_trigger(dev, &it)) < 0) {
		perror("ioctl");
		exit(1);
	}

	if((n=read(comedi_fileno(dev),data,N_SAMPLES*sizeof(sampl_t)))<0){
		perror("read");
		exit(1);
	}
	printf("number of samples read=%d\ndata[0]=%d\ndata[N-1]=%d\n",
		n/sizeof(sampl_t),data[0],data[N_SAMPLES-1]);

	return 0;
}

