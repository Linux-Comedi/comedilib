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

#include <libinternal.h>

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


#define BUFSZ 100

int comedi_timed_1chan(comedi_t *dev,unsigned int subd,unsigned int chan,unsigned int range,
	unsigned int aref,double freq,unsigned int n_samples,double *data)
{
	comedi_trig t;
	double act_freq;
	sampl_t *buffer;
	comedi_range *the_range;
	unsigned int maxdata;
	int i,n,m;
	
	if(!valid_chan(dev,subd,chan))return -1;
	if(!data)return -1;
	
	memset(&t,0,sizeof(t));
	
	/* check range */

	chan=CR_PACK(chan,range,aref);

	t.subdev=subd;
	t.mode=2;
	t.n_chan=1;
	t.chanlist=&chan;
	t.n=n_samples;
	comedi_get_timer(dev,subd,freq,&t.trigvar,&act_freq);
	t.trigvar1=1;
	
	the_range=comedi_get_range(dev,subd,chan,range);
	maxdata=comedi_get_maxdata(dev,subd,chan);
	
	buffer=malloc(sizeof(sampl_t)*BUFSZ);
	if(!buffer)return -1;

	comedi_trigger(dev,&t);
	n=0;
	while(n<n_samples){
		m=n_samples-n;
		if(m>BUFSZ)m=BUFSZ;
		if((m=read(dev->fd,buffer,m*sizeof(sampl_t)))<0){
			/* ack! */
			return -1;
		}
		m/=sizeof(sampl_t);
		for(i=0;i<m;i++){
			data[n+i]=comedi_to_phys(buffer[i],the_range,maxdata);
		}
		n+=m;
	}

	free(buffer);

	return 0;
}

