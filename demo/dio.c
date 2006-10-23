/*
 * Digital I/O example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a digital I/O subdevice.  Not just
 *    a 'digital input' or 'digital output' subdevice, but one in
 *    which the channels can be configured between input and output.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"


comedi_t *device;

int main(int argc, char *argv[])
{
	int ret;
	int stype;

	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	stype = comedi_get_subdevice_type(device,subdevice);
	if(stype!=COMEDI_SUBD_DIO){
		printf("%d is not a digital I/O subdevice\n",subdevice);
		exit(0);
	}

	printf("configuring pin %d or subdevice %d ", channel, subdevice);
	if(value)
	{
		printf("for output.\n");
		ret=comedi_dio_config(device,subdevice,channel, COMEDI_OUTPUT);
	}else
	{
		printf("for input.\n");
		ret=comedi_dio_config(device,subdevice,channel, COMEDI_INPUT);
	}
	return 0;
}

