/*
    lib/range.c
    comedi library routines for voltage ranges

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

#define __USE_GNU

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


/* sometimes we can't find a definition of NAN */

#ifndef	NAN
#define	NAN \
  (__extension__ ((union { unsigned char __c[8];			      \
			   double __d; })				      \
		  { { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f } }).__d)
#endif


static enum comedi_oor_behavior comedi_oor_is_nan = COMEDI_OOR_NAN;

enum comedi_oor_behavior comedi_set_global_oor_behavior(
	enum comedi_oor_behavior behavior)
{
	int old_behavior=comedi_oor_is_nan;

	comedi_oor_is_nan=behavior;

	return old_behavior;
}


double comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata)
{
	double x;

	if(!rng)return NAN;
	if(!maxdata)return NAN;

	if(comedi_oor_is_nan==COMEDI_OOR_NAN && (data==0 || data==maxdata))
		return NAN;

	x=data;
	x/=maxdata;
	x*=(rng->max-rng->min);
	x+=rng->min;

	return x;
}

lsampl_t comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata)
{
	double s;

	if(!rng)return 0;
	if(!maxdata)return 0;

	s=(data-rng->min)/(rng->max-rng->min)*maxdata;
	if(s<0)return 0;
	if(s>maxdata)return maxdata;

	return (lsampl_t)(floor(s+0.5));
}

int comedi_find_range(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int unit,double min,double max)
{
	unsigned int range_type;
	int best;
	comedi_range *range_ptr,*best_ptr;
	int i;
	
	if(!valid_chan(it,subd,chan))return -1;

	range_type=comedi_get_rangetype(it,subd,chan);
	best=-1;
	best_ptr=NULL;
	for(i=0;i<RANGE_LENGTH(range_type);i++){
		range_ptr=comedi_get_range(it,subd,chan,i);
		if(range_ptr->min<=min && range_ptr->max>=max){
			if(best<0 || (range_ptr->max-range_ptr->min) < 
			   (best_ptr->max-best_ptr->min)){
				best=i;
				best_ptr=range_ptr;
			}
		}
	}
	return best;
}

int comedi_get_n_ranges(comedi_t *it,unsigned int subd,unsigned int chan)
{
	unsigned int range_type;

	if(!valid_chan(it,subd,chan))return -1;

	range_type=comedi_get_rangetype(it,subd,chan);
	return RANGE_LENGTH(range_type);
}

