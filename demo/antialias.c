/*
 * Antialiasing Analog Output Demo
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/* Not functional */

/*
 * Requirements: an analog output subdevice that is capable
 *     of ansynchronous output.
 *
 * Normally, the resolution of analog output channels is limited by
 * the resolution of the D/A converter.  However, if you limit the
 * bandwith of the D/A converter by using a low-pass filter, you
 * can trade some of the bandwidth for additional resolution.  This
 * is done by changing the output rapidly between two adjacent
 * values: a signal of an alternating 0,1,0,1,0,1 sequence will
 * look like 0.5 after an appropriate low-pass filter.
 *
 * The disadvantage, of course, is that you lose bandwidth.  Worse,
 * the simple technique demonstrated here will cause predictable
 * noise in the stop band.  More complicated techniques will allow
 * you to tune the spectrum of the noise in the stop band.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"

void ao_antialias(unsigned int data);

comedi_t *device;

int main(int argc, char *argv[])
{
	lsampl_t data;
	int ret;

	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	data = value; 
	if(verbose){
		printf("writing %d to device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			data,filename,subdevice,channel,range,aref);
	}

	ret=comedi_data_write(device,subdevice,channel,range,aref,data);
	if(ret<0){
		comedi_perror(filename);
		exit(0);
	}

	printf("%d\n",data);

	ao_antialias((1000<<16)+1000);

	return 0;
}


void ao_antialias(unsigned int data)
{
	unsigned int hibits=data>>16;
	unsigned int lobits=data&0xffff;
	int i;
	unsigned int acc;

	acc=0;
	for(i=0;i<100;i++){
		acc+=lobits;
		printf("%d\n",hibits+(acc>>16));
		acc&=0xffff;
	}
}

