/*
 * Tutorial example #2
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <stdio.h>		/* for printf() */
#include <comedilib.h>

int subdev = 0;			/* change this to your input subdevice */
int chan = 0;			/* change this to your channel */
int range = 0;			/* more on this later */
int aref = AREF_GROUND;		/* more on this later */

int main(int argc, char *argv[])
{
	comedi_t *cf;
	lsampl_t data;
	int maxdata;
	double volts;
	comedi_range *cr;

	cf = comedi_open("/dev/comedi0");
	maxdata = comedi_get_maxdata(cf, subdev, chan);
	cr = comedi_get_range(cf, subdev, chan, range);
	comedi_data_read(cf, subdev, chan, range, aref, &data);
	volts = comedi_to_phys(data, cr, maxdata);
	printf("%d %g\n", data, volts);

	return 0;
}
