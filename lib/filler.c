/*
    lib/filler.c
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


/* these functions download information from the comedi module. */



int get_subdevices(comedi_t *it)
{
	int i,j;
	int ret;
	comedi_subdinfo *s;
	subdevice *r;

	s=malloc(sizeof(comedi_subdinfo)*it->n_subdevices);
	ret=ioctl_subdinfo(it->fd,s);
	debug_int(ret);

	r=it->subdevices=realloc(it->subdevices,
		sizeof(subdevice)*it->n_subdevices);
	debug_ptr(r);
	memset(r,0,sizeof(subdevice)*it->n_subdevices);

	for(i=0;i<it->n_subdevices;i++){
		r[i].type	= s[i].type;
		if(r[i].type==COMEDI_SUBD_UNUSED)continue;
		r[i].n_chan	= s[i].n_chan;
		r[i].subd_flags	= s[i].subd_flags;
		r[i].timer_type	= s[i].timer_type;
		r[i].len_chanlist = s[i].len_chanlist;
		r[i].maxdata	= s[i].maxdata;
		r[i].flags	= s[i].flags;
		r[i].range_type = s[i].range_type;

		if(r[i].subd_flags&SDF_FLAGS){
			r[i].flags_list=malloc(sizeof(*r[i].flags_list)*r[i].n_chan);
			debug_ptr(r[i].flags_list);
		}
		if(r[i].subd_flags&SDF_MAXDATA){
			r[i].maxdata_list=malloc(sizeof(*r[i].maxdata_list)*r[i].n_chan);
			debug_ptr(r[i].maxdata_list);
		}
		if(r[i].subd_flags&SDF_RANGETYPE){
			r[i].range_type_list=malloc(sizeof(*r[i].range_type_list)*r[i].n_chan);
			debug_ptr(r[i].range_type_list);
		}
		ret=ioctl_chaninfo(it->fd,i,r[i].maxdata_list,r[i].flags_list,r[i].range_type_list);
		debug_int(ret);

		if(r[i].subd_flags&SDF_RANGETYPE){
			r[i].rangeinfo_list=malloc(sizeof(*r[i].rangeinfo_list)*r[i].n_chan);
			debug_ptr(r[i].rangeinfo_list);
			for(j=0;j<r[i].n_chan;j++){
				r[i].rangeinfo_list[j]=get_rangeinfo(it->fd,r[i].range_type_list[j]);
			}
		}else{
			r[i].rangeinfo=get_rangeinfo(it->fd,r[i].range_type);
		}
	}

	free(s);

	return 0;
}

comedi_range *get_rangeinfo(int fd,unsigned int range_type)
{
	comedi_krange *kr;
	comedi_range *r;
	int i;

	kr=malloc(sizeof(comedi_krange)*RANGE_LENGTH(range_type));
	r=malloc(sizeof(comedi_range)*RANGE_LENGTH(range_type));

	ioctl_rangeinfo(fd,range_type,kr);

	for(i=0;i<RANGE_LENGTH(range_type);i++){
		r[i].min=kr[i].min*1e-6;
		r[i].max=kr[i].max*1e-6;
		r[i].unit=RF_UNIT(kr[i].flags);
	}
	free(kr);

	return r;
}


