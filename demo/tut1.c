/*
 * Tutorial example #1
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <stdio.h>	/* for printf() */
#include <comedilib.h>

int subdev = 0;		/* change this to your input subdevice */
int chan = 0;		/* change this to your channel */
int range = 0;		/* more on this later */
int aref = AREF_GROUND;	/* more on this later */

int main(int argc,char *argv[])
{
	comedi_t *it;
	int chan=0;
	lsampl_t data;
	
	it=comedi_open("/dev/comedi0");
	
	comedi_data_read(it,subdev,chan,range,aref,&data);
	
	printf("%d\n",data);
	
	return 0;
}

