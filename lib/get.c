/*
    lib/get.c
    comedi library routines

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1997-8 David A. Schleef <ds@stm.lbl.gov>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <comedi.h>
#include <string.h>

#include <libinternal.h>


int comedi_get_n_subdevices(comedi_t *it)
{
	if(!valid_dev(it))
		return -1;

	return it->n_subdevices;
}

int comedi_get_version_code(comedi_t *it)
{
	if(!valid_dev(it))
		return -1;

	return it->devinfo.version_code;
}

char *comedi_get_driver_name(comedi_t *it)
{
	if(!valid_dev(it))
		return NULL;

	return it->devinfo.driver_name;
}

char *comedi_get_board_name(comedi_t *it)
{
	if(!valid_dev(it))
		return NULL;

	return it->devinfo.board_name;
}

int comedi_get_subdevice_type(comedi_t *it,unsigned int subd)
{
	if(!valid_dev(it))
		return -1;

	return it->subdevices[subd].type;
}

int comedi_find_subdevice_by_type(comedi_t *it,int type,unsigned int subd)
{
	if(!valid_subd(it,subd))
		return -1;

	for(;subd<it->n_subdevices;subd++){
		if(it->subdevices[subd].type==type)
			return subd;
	}
	return -1;
}


int comedi_get_n_channels(comedi_t *it,unsigned int subd)
{
	if(!valid_subd(it,subd))
		return -1;

	return it->subdevices[subd].n_chan;
}


/* */

lsampl_t comedi_get_maxdata(comedi_t *it,unsigned int subdevice,unsigned int chan)
{
	if(!valid_chan(it,subdevice,chan))
		return 0;

	if(it->subdevices[subdevice].maxdata_list)
		return it->subdevices[subdevice].maxdata_list[chan];

	return it->subdevices[subdevice].maxdata;
}

int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,unsigned int chan)
{
	if(!valid_chan(it,subdevice,chan))
		return -1;

	if(it->subdevices[subdevice].range_type_list)
		return it->subdevices[subdevice].range_type_list[chan];

	return it->subdevices[subdevice].range_type;
}


comedi_range * comedi_get_range(comedi_t *it,unsigned int subdevice,unsigned int chan,unsigned int range)
{
	int range_type;

	if(!valid_chan(it,subdevice,chan))
		return NULL;

	range_type=comedi_get_rangetype(it,subdevice,chan);

	if(range>=RANGE_LENGTH(range_type))
		return NULL;

	if(it->subdevices[subdevice].rangeinfo_list)
		return it->subdevices[subdevice].rangeinfo_list[chan]+range;

	return it->subdevices[subdevice].rangeinfo+range;
}



