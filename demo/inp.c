/*
 * A very small one-shot input demo
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
   A little input demo
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"

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

	if(verbose_flag){
		printf("measuring device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			filename,subdevice,channel,range,aref);
	}

	ret=comedi_data_read(device,subdevice,channel,range,aref,&data);
	if(ret<0){
		comedi_perror(filename);
		exit(0);
	}

	printf("%d\n",data);

	return 0;
}

