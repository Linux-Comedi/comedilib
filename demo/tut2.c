/*
 * Tutorial example #2
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2008 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <stdio.h>		/* for printf() */
#include <stdlib.h>
#include <comedilib.h>
#include <ctype.h>
#include <math.h>

int subdev = 0;			/* change this to your input subdevice */
int chan = 0;			/* change this to your channel */
int range = 0;			/* more on this later */
int aref = AREF_GROUND;		/* more on this later */
const char filename[] = "/dev/comedi0";

int main(int argc, char *argv[])
{
	comedi_t *device;
	lsampl_t data;
	double physical_value;
	int retval;
	comedi_range *range_info;
	lsampl_t maxdata;

	device = comedi_open(filename);
	if (device == NULL) {
		comedi_perror(filename);
		return 1;
	}

	retval = comedi_data_read(device, subdev, chan, range, aref,
				  &data);
	if (retval < 0) {
		comedi_perror(filename);
		return 1;
	}

	comedi_set_global_oor_behavior(COMEDI_OOR_NAN);
	range_info = comedi_get_range(device, subdev, chan, range);
	maxdata = comedi_get_maxdata(device, subdev, chan);
	printf("[0,%d] -> [%g,%g]\n", maxdata,
	       range_info->min, range_info->max);
	physical_value = comedi_to_phys(data, range_info, maxdata);
	if (isnan(physical_value)) {
		printf("Out of range [%g,%g]",
		       range_info->min, range_info->max);
	} else {
		printf("%g", physical_value);
		switch(range_info->unit) {
		case UNIT_volt: printf(" V"); break;
		case UNIT_mA: printf(" mA"); break;
		case UNIT_none: break;
		default: printf(" (unknown unit %d)",
				range_info->unit);
		}
		printf(" (%lu in raw units)\n", (unsigned long)data);
	}
	return 0;
}
