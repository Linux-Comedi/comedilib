/*
	A little input demo for mode 1

	Mode 1 uses a timer to acquire samples at regular intervals.
	It scans through the channel list, and then repeats.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define N_SAMPLES	100000
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
	double actual_freq;
	void *data_ptr;
	int n_left;

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	it.subdev = 0;
	it.mode = 1;
	it.flags = 0;
	it.n_chan = N_CHANS;
	it.chanlist = chan;
	it.data = data;
	it.n = N_SAMPLES/N_CHANS;
	it.trigsrc = 0;

	/* convert the frequency into a timer value */
	comedi_get_timer(dev,subdevice,freq,&it.trigvar,&actual_freq);
	printf("primary actual frequency=%g timer value=%d\n",actual_freq,it.trigvar);

	/* pack the channel list */
	for(i=0;i<N_CHANS;i++){
		chan[i] = CR_PACK(channels[i], range, aref);
	}

	if ((err = comedi_trigger(dev, &it)) < 0) {
		perror("ioctl");
		exit(1);
	}

	data_ptr=data;
	n_left=N_SAMPLES*sizeof(sampl_t);
	while(n_left>0){
		if((n=read(comedi_fileno(dev),data_ptr,n_left))<0){
			perror("read");
			exit(1);
		}
		printf("read %d\n",n);
		n_left-=n;
		data_ptr+=n;
	}
	printf("number of samples read=%d\ndata[0]=%d\ndata[N-1]=%d\n",
		n/sizeof(sampl_t),data[0],data[N_SAMPLES-1]);

	for(i=0;i<N_SAMPLES;i++){
		fprintf(stderr,"%d\n",data[i]);
	}

	return 0;
}

