/*
   A little input demo for mode 3

   Mode 3 uses an external trigger to repeatedly trigger
   acquisition of samples.  If you select n_chan>1, then
   a different channel is captured at each external trigger.

   If you have multiple external trigger lines, the
   particular trigger line is selected by trigval.

 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define N_SAMPLES	100
#define N_CHANS		4

int subdevice = 0;
int channels[N_CHANS] = { 0, 1, 2, 3 };
double freq = 100000;
int range = 0;
int aref = 3;
int external_trigger_number = 0;

sampl_t data[N_SAMPLES];


int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_trig it;
	int err;
	int n,i;
	comedi_t *dev;
	unsigned int chan[N_CHANS];

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	it.subdev = 0;
	it.mode = 3;
	it.flags = 0;
	it.n_chan = N_CHANS;
	it.chanlist = chan;
	it.data = data;
	it.n = N_SAMPLES/N_CHANS;
	it.trigsrc = 0;

	/* external trigger number */
	it.trigvar = external_trigger_number;

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

