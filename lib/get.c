/*
    lib/get.c
    functions to return information about comedi devices

    COMEDILIB - Linux Control and Measurement Device Interface Library
    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, version 2.1
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
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


EXPORT_SYMBOL(comedi_get_n_subdevices,0.7.18);
int comedi_get_n_subdevices(comedi_t *it)
{
	if(!valid_dev(it))
		return -1;

	return it->n_subdevices;
}

EXPORT_SYMBOL(comedi_get_version_code,0.7.18);
int comedi_get_version_code(comedi_t *it)
{
	if(!valid_dev(it))
		return -1;

	return it->devinfo.version_code;
}

EXPORT_SYMBOL(comedi_get_driver_name,0.7.18);
char *comedi_get_driver_name(comedi_t *it)
{
	if(!valid_dev(it))
		return NULL;

	return it->devinfo.driver_name;
}

EXPORT_SYMBOL(comedi_get_board_name,0.7.18);
char *comedi_get_board_name(comedi_t *it)
{
	if(!valid_dev(it))
		return NULL;

	return it->devinfo.board_name;
}

EXPORT_SYMBOL(comedi_get_subdevice_flags,0.7.18);
int comedi_get_subdevice_flags(comedi_t *it,unsigned int subd)
{
	if(!valid_dev(it))
		return 0;

	return it->subdevices[subd].subd_flags;
}

EXPORT_SYMBOL(comedi_get_subdevice_type,0.7.18);
int comedi_get_subdevice_type(comedi_t *it,unsigned int subd)
{
	if(!valid_dev(it))
		return -1;

	return it->subdevices[subd].type;
}

EXPORT_SYMBOL(comedi_get_subdevice_type,0.7.18);
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

EXPORT_SYMBOL(comedi_get_read_subdevice,0.7.18);
int comedi_get_read_subdevice(comedi_t *dev)
{
	if(!valid_dev(dev))
		return -1;

	return dev->devinfo.read_subdevice;
}

EXPORT_SYMBOL(comedi_get_write_subdevice,0.7.18);
int comedi_get_write_subdevice(comedi_t *dev)
{
	if(!valid_dev(dev))
		return -1;

	return dev->devinfo.write_subdevice;
}

EXPORT_SYMBOL(comedi_get_n_channels,0.7.18);
int comedi_get_n_channels(comedi_t *it,unsigned int subd)
{
	if(!valid_subd(it,subd))
		return -1;

	return it->subdevices[subd].n_chan;
}


/* */

EXPORT_SYMBOL(comedi_get_maxdata,0.7.18);
lsampl_t comedi_get_maxdata(comedi_t *it,unsigned int subdevice,unsigned int chan)
{
	if(!valid_chan(it,subdevice,chan))
		return 0;

	if(it->subdevices[subdevice].maxdata_list)
		return it->subdevices[subdevice].maxdata_list[chan];

	return it->subdevices[subdevice].maxdata;
}

EXPORT_SYMBOL(comedi_maxdata_is_chan_specific,0.7.18);
int comedi_maxdata_is_chan_specific(comedi_t *it,unsigned int subdevice)
{
	if(it->subdevices[subdevice].maxdata_list)
		return 1;
	return 0;
}

EXPORT_SYMBOL(comedi_get_rangetype,0.7.18);
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,unsigned int chan)
{
	if(!valid_chan(it,subdevice,chan))
		return -1;

	if(it->subdevices[subdevice].range_type_list)
		return it->subdevices[subdevice].range_type_list[chan];

	return it->subdevices[subdevice].range_type;
}


EXPORT_SYMBOL(comedi_get_range,0.7.18);
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


