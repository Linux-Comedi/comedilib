/*
   This demo uses an analog output subdevice in timed
   mode (mode 2) to generate a waveform.  The waveform
   in this example is a sine wave (surprise!), but this
   can be easily changed to make a general function
   generator.

   The function generation algorithm is the same as
   what is typically used in digital function generators.
   A 32-bit accumulator is incremented by a phase factor,
   which is the amount (in radians) that the generator
   advances each time step.  The accumulator is then
   shifted right by 20 bits, to get a 12 bit offset into
   a lookup table.  The value in the lookup table at
   that offset is then put into a buffer for output to
   the DAC.

   [ Actually, the accumulator is only 26 bits, for some
   reason.  I'll fix this sometime. ]

   On the comedi side of things, the setup for mode 2
   is similar to analog input, except for the TRIG_WRITE
   flag.  Once you have issued the command, comedi then
   expects you to keep the buffer full of data to output
   to the DAC.  This is done by write().  Since there
   may be a delay between the ioctl() and a subsequent
   write(), you should fill the buffer using write() before
   you call ioctl(), as is done here.

   Also NOTE!  The lseek() to offset 1 is used to tell
   comedi that you want to write to subdevice 1.  This
   is not needed for analog input, since AI is usually on
   subdevice 0.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>

#define dds_init dds_init_pseudocycloid


/* frequency of the sine wave to output */
double waveform_frequency	= 100.0;

/* update rate for the DAC, typically much higher than
   the frequency of the sine wave. */
double update_frequency		= 50000.0;

/* peak-to-peak amplitude, in DAC units (i.e., 0-4095) */
double amplitude		= 4000;

/* offset, in DAC units */
double offset			= 2048;

/* This is the size of chunks we deal with when creating and
   outputting data.  This *could* be 1, but that would be
   inefficient */
#define BUF_LEN		4096


#define N_SCANS		0
#define N_CHANS		1

int subdevice;
int channels[] = { 0 };
int range = 0;
int aref = AREF_GROUND;
int external_trigger_number = 0;

sampl_t data[BUF_LEN];

void dds_output(sampl_t *buf,int n);
void dds_init_sine(void);
void dds_init_pseudocycloid(void);

int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_trig it;
	int err;
	int n,m,i;
	int total=0;
	comedi_t *dev;
	double actual_freq;
	unsigned int chan[N_CHANS];

	if(argc>=2){
		waveform_frequency=atof(argv[1]);
	}

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	subdevice = comedi_find_subdevice_by_type(dev,COMEDI_SUBD_AO,0);

	it.subdev = subdevice;
	it.mode = 2;
	it.flags = TRIG_WRITE;
	it.n_chan = N_CHANS;
	it.chanlist = chan;
	it.data = NULL;
	it.n = N_SCANS;
	it.trigsrc = 0;

	/* convert the frequency into a timer value */
	comedi_get_timer(dev,subdevice,update_frequency,&it.trigvar,&actual_freq);
	fprintf(stderr,"primary actual frequency=%g timer value=%d\n",actual_freq,it.trigvar);

	/* pack the channel list */
	for(i=0;i<N_CHANS;i++){
		chan[i] = CR_PACK(channels[i], range, aref);
	}

	dds_init();

	dds_output(data,BUF_LEN);
	dds_output(data,BUF_LEN);

	lseek(comedi_fileno(dev),subdevice,SEEK_SET);
	m=write(comedi_fileno(dev),data,BUF_LEN*sizeof(sampl_t));
	perror("write");
	printf("m=%d\n",m);


	if ((err = comedi_trigger(dev, &it)) < 0) {
		perror("ioctl");
		exit(1);
	}
	while(1){
		dds_output(data,BUF_LEN);
		n=BUF_LEN*sizeof(sampl_t);
		while(n>0){
			m=write(comedi_fileno(dev),(void *)data+(BUF_LEN*sizeof(sampl_t)-n),n);
			if(m<0){
				perror("write");
				exit(0);
			}
			//printf("m=%d\n",m);
			n-=m;
		}
		total+=BUF_LEN;
		//printf("%d\n",total);
	}

	return 0;
}



#define WAVEFORM_SHIFT 16
#define WAVEFORM_LEN (1<<WAVEFORM_SHIFT)
#define WAVEFORM_MASK (WAVEFORM_LEN-1)


sampl_t waveform[WAVEFORM_LEN];

unsigned int acc;
unsigned int adder;

void dds_init(void)
{
	int i;

	adder=waveform_frequency/update_frequency*(1<<16)*(1<<WAVEFORM_SHIFT);

	dds_init_sine();

	/* this is due to a bug in the NI-E driver */
	if(range){
		for(i=0;i<WAVEFORM_LEN;i++){
			waveform[i]^=0x800;
		}
	}
}

void dds_init_sine(void)
{
	int i;

	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(offset+0.5*amplitude*cos(i*2*M_PI/WAVEFORM_LEN));
	}
}

/* Yes, I know this is not the proper equation for a
   cycloid.  Fix it. */
void dds_init_cycloid(void)
{
	int i;
	double t;

	for(i=0;i<WAVEFORM_LEN/2;i++){
		t=2*((double)i)/WAVEFORM_LEN;
		waveform[i]=rint(offset+amplitude*sqrt(1-4*t*t));
	}
	for(i=WAVEFORM_LEN/2;i<WAVEFORM_LEN;i++){
		t=2*(1-((double)i)/WAVEFORM_LEN);
		waveform[i]=rint(offset+amplitude*sqrt(1-t*t));
	}
}

void dds_init_sawtooth(void)
{
	int i;

	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(offset+amplitude*((double)i)/WAVEFORM_LEN);
	}
}

void dds_output(sampl_t *buf,int n)
{
	int i;
	sampl_t *p=buf;

	for(i=0;i<n;i++){
		*p=waveform[(acc>>16)&WAVEFORM_MASK];
		p++;
		acc+=adder;
	}
}

