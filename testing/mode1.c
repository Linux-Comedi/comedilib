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
#include <math.h>
#include <sys/time.h>
#include <string.h>

#define BUFSZ		1000
#define N_CHANS		1000

extern int verbose_flag;
extern int subdevice;
extern int range;
extern int channel;
extern int aref;
extern char *filename;

comedi_t *device;

void parse_options(int argc,char *argv[]);

sampl_t data[BUFSZ];

unsigned int chanlist[N_CHANS];

int ai_mode1_test(double freq,int n_chans,int n_scans);

int main(int argc, char *argv[])
{
	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	subdevice=comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0);
	if(subdevice<0){
		printf("no analog input subdevice found\n");
		exit(0);
	}

while(1){
	printf("Testing mode 1:\n");

	printf("10 hz, 1 chan\n");
	ai_mode1_test(10.0,1,10);

	printf("100 hz, 1 chan\n");
	ai_mode1_test(100.0,1,100);

	printf("1000 hz, 1 chan\n");
	ai_mode1_test(1000.0,1,1000);

	printf("10000 hz, 1 chan\n");
	ai_mode1_test(10000.0,1,10000);

	printf("100000 hz, 1 chan\n");
	ai_mode1_test(100000.0,1,100000);

	printf("100000 hz, 2 chan\n");
	ai_mode1_test(100000.0,2,50000);

	printf("100000 hz, 16 chans\n");
	ai_mode1_test(100000.0,16,100000/16);
}


	return 0;
}

int ai_mode1_test(double freq,int n_chans,int n_scans)
{
	comedi_trig it;
	int err;
	int n,i;
	double actual_freq;
	void *data_ptr;
	int n_left;
	int maxchan;
	struct timeval start,stop;
	int m;

	memset(&it,0,sizeof(it));
	it.subdev = subdevice;
	it.mode = 1;
	it.n_chan = n_chans;
	it.chanlist = chanlist;
	it.data = data;
	it.n = n_scans;

	maxchan = comedi_get_n_channels(device,subdevice);

	/* pack the channel list */
	for(i=0;i<n_chans;i++){
		chanlist[i] = CR_PACK(i%maxchan, range, aref);
	}

	comedi_get_timer(device,subdevice,freq,&it.trigvar,&actual_freq);
	printf("primary actual frequency=%g timer value=%d\n",actual_freq,it.trigvar);

	gettimeofday(&start,NULL);

	if ((err = comedi_trigger(device, &it)) < 0) {
		perror("ioctl");
	}

	data_ptr=data;
	n_left=n_scans*n_chans*sizeof(sampl_t);
	while(n_left>0){
		m=n_left;
		if(m>=BUFSZ)m=BUFSZ;
		if((n=read(comedi_fileno(device),data_ptr,m))<0){
			perror("read");
			exit(1);
		}
		//printf("read %d\n",n);
		n_left-=n;
		//data_ptr+=n;
	}

	gettimeofday(&stop,NULL);

	if(stop.tv_usec<=start.tv_usec){
		stop.tv_usec+=1000000;
		stop.tv_sec--;
	}
	stop.tv_sec-=start.tv_sec;
	stop.tv_usec-=start.tv_usec;

	printf("actual time elapsed: %d.%06d s.\n",(int)stop.tv_sec,(int)stop.tv_usec);
	printf("expected time: %g\n",n_scans*n_chans/actual_freq);

	return 0;
}

