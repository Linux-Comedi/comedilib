/*
 * Asynchronous Analog Output Example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * Requirements: Analog output device capable of
 *    asynchronous commands.
 *
 * This demo uses an analog output subdevice with an
 * asynchronous command to generate a waveform.  The
 * waveform in this example is a sine wave (surprise!),
 * but this can be easily changed to make a generic
 * function generator.
 *
 * The function generation algorithm is the same as
 * what is typically used in digital function generators.
 * A 32-bit accumulator is incremented by a phase factor,
 * which is the amount (in radians) that the generator
 * advances each time step.  The accumulator is then
 * shifted right by 20 bits, to get a 12 bit offset into
 * a lookup table.  The value in the lookup table at
 * that offset is then put into a buffer for output to
 * the DAC.
 *
 * [ Actually, the accumulator is only 26 bits, for some
 * reason.  I'll fix this sometime. ]
 *
 * On the Comedi side of things, the setup for mode 2
 * is similar to analog input, except for the TRIG_WRITE
 * flag.  Once you have issued the command, comedi then
 * expects you to keep the buffer full of data to output
 * to the DAC.  This is done by write().  Since there
 * may be a delay between the comedi_command() and a subsequent
 * write(), you should fill the buffer using write() before
 * you call comedi_command(), as is done here.
 *
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include "examples.h"


/* frequency of the sine wave to output */
double waveform_frequency	= 100.0;

/* peak-to-peak amplitude, in DAC units (i.e., 0-4095) */
double amplitude		= 4000;

/* offset, in DAC units */
double offset			= 2048;

/* This is the size of chunks we deal with when creating and
   outputting data.  This *could* be 1, but that would be
   inefficient */
#define BUF_LEN		4096

int subdevice;
int external_trigger_number = 0;

sampl_t data[BUF_LEN];

void dds_output(sampl_t *buf,int n);
void dds_init(void);

/* This define determines which waveform to use. */
#define dds_init_function dds_init_sine

void dds_init_sine(void);
void dds_init_pseudocycloid(void);
void dds_init_sawtooth(void);

int main(int argc, char *argv[])
{
	comedi_cmd cmd;
	comedi_insn insn;
	int err;
	int n,m;
	int total=0;
	comedi_t *dev;
	unsigned int chanlist[16];

	parse_options(argc,argv);

	/* Force n_chan to be 1 */
	n_chan = 1;

	if(value){
		waveform_frequency = value;
	}

	dev = comedi_open(filename);
	if(dev == NULL){
		fprintf(stderr, "error opening %s\n", filename);
		return -1;
	}
	subdevice = comedi_find_subdevice_by_type(dev,COMEDI_SUBD_AO,0);

	memset(&cmd,0,sizeof(cmd));
	cmd.subdev = subdevice;
	cmd.flags = 0;
	cmd.start_src = TRIG_INT;
	cmd.start_arg = 0;
	cmd.scan_begin_src = TRIG_TIMER;
	cmd.scan_begin_arg = 1e9/freq;
	cmd.convert_src = TRIG_NOW;
	cmd.convert_arg = 0;
	cmd.scan_end_src = TRIG_COUNT;
	cmd.scan_end_arg = n_chan;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;

	cmd.chanlist = chanlist;
	cmd.chanlist_len = n_chan;

	chanlist[0] = CR_PACK(channel,range,aref);
	chanlist[1] = CR_PACK(channel+1,range,aref);

	dds_init();

	dds_output(data,BUF_LEN);
	dds_output(data,BUF_LEN);

	if ((err = comedi_command(dev, &cmd)) < 0) {
		comedi_perror("comedi_command");
		exit(1);
	}

	m=write(comedi_fileno(dev),data,BUF_LEN*sizeof(sampl_t));
	perror("write");
	printf("m=%d\n",m);
	
	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_INTTRIG;
	insn.subdev = subdevice;
	comedi_do_insn(dev, &insn);

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

	adder=waveform_frequency/freq*(1<<16)*(1<<WAVEFORM_SHIFT);

	dds_init_function();

	/* this is due to a bug in the NI-E driver */
	if(range){
		for(i=0;i<WAVEFORM_LEN;i++){
			waveform[i]^=0x800;
		}
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


void dds_init_sine(void)
{
	int i;

	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(offset+0.5*amplitude*cos(i*2*M_PI/WAVEFORM_LEN));
	}
}

/* Yes, I know this is not the proper equation for a
   cycloid.  Fix it. */
void dds_init_pseudocycloid(void)
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

