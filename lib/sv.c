/*
    lib/sv.c
    comedi library routines - sv section

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



int sv_measure_l(comedi_sv_t *it,double *data);
int sv_measure_s(comedi_sv_t *it,double *data);


int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan)
{
	if(!valid_chan(dev,subd,chan))return -1;
	if(!it)return -1;

	memset(it,0,sizeof(*it));

	it->dev=dev;
	it->subdevice=subd;
	it->chan=chan;
	it->n=100;

	return comedi_sv_update(it);
}

int comedi_sv_update(comedi_sv_t *it)
{
	if(!it)return -1;
	if(!valid_chan(it->dev,it->subdevice,it->chan))return -1;

	it->maxdata=comedi_get_maxdata(it->dev,it->subdevice,it->chan);

	/* check current range policy */
	/* check n */

	return 0;
}

int comedi_sv_measure(comedi_sv_t *it,double *data)
{
	if(it->dev->subdevices[it->subdevice].subd_flags & SDF_LSAMPL){
		return sv_measure_l(it,data);
	}else{
		return sv_measure_s(it,data);
	}
}

int sv_measure_l(comedi_sv_t *it,double *data)
{
	comedi_trig t;
	int ret=0;
	lsampl_t *val;
	unsigned int chan;
	comedi_range *rng;
	double sum;
	int i;
	int n;

	val=malloc(sizeof(*val)*it->n);

	chan=CR_PACK(it->chan,it->range,it->aref);
	
	t.subdev=it->subdevice;
	t.mode=0;
	t.flags=TRIG_DITHER;
	t.n_chan=1;
	t.chanlist=&chan;
	t.trigsrc=0;
	t.trigvar=0;
	t.trigvar1=0;

	rng=comedi_get_range(it->dev,it->subdevice,it->chan,it->range);

	for(n=0;n<it->n;){
		t.data=(void *)(val+n);
		t.n=it->n-n;
		i=ioctl_trigger(it->dev->fd,&t);
		if(i<=0){
			ret=i;
			goto out;
		}
		n+=i;
	}

	sum=0;
	for(i=0;i<it->n;i++){
		sum+=comedi_to_phys(val[i],rng,it->maxdata);
	}
	*data=sum/it->n;

out:
	free(val);

	return ret;
}

/* yes, these functions are _almost_ exactly the same... */

int sv_measure_s(comedi_sv_t *it,double *data)
{
	comedi_trig t;
	int ret=0;
	sampl_t *val;
	unsigned int chan;
	comedi_range *rng;
	double sum;
	int i;
	int n;

	val=malloc(sizeof(*val)*it->n);

	chan=CR_PACK(it->chan,it->range,it->aref);
	
	t.subdev=it->subdevice;
	t.mode=0;
	t.flags=TRIG_DITHER;
	t.n_chan=1;
	t.chanlist=&chan;
	t.data=val;
	t.n=it->n;
	t.trigsrc=0;
	t.trigvar=0;
	t.trigvar1=0;

	rng=comedi_get_range(it->dev,it->subdevice,it->chan,it->range);

	for(n=0;n<it->n;){
		t.data=val+n;
		t.n=it->n-n;
		i=ioctl_trigger(it->dev->fd,&t);
		if(i<=0){
			ret=i;
			goto out;
		}
		n+=i;
	}

	sum=0;
	for(i=0;i<it->n;i++){
		sum+=comedi_to_phys(val[i],rng,it->maxdata);
	}
	*data=sum/it->n;

out:
	free(val);

	return ret;
}







