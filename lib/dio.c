/*
    lib/dio.c
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
#include <errno.h>
#include <comedi.h>
#include <string.h>

#include <libinternal.h>


int comedi_dio_config(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int io)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;
	
	s=it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO)
		return -1;

	if(io!=COMEDI_INPUT && io!=COMEDI_OUTPUT)
		return -1;

	if(s->has_insn){
		comedi_insn insn;
		lsampl_t data;
		
		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_CONFIG;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);
		data=io;

		return comedi_do_insn(it,&insn);
	}else
	{
		comedi_trig trig;
		lsampl_t data=io;

		memset(&trig,0,sizeof(trig));
		trig.flags=TRIG_CONFIG|TRIG_WRITE;
		trig.n_chan=1;
		trig.n=1;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=(sampl_t *)&data;

		return comedi_trigger(it,&trig);
	}
}

int comedi_dio_read(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int *val)
{
	subdevice *s;
	int ret;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s = it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO &&
	   s->type!=COMEDI_SUBD_DO &&
	   s->type!=COMEDI_SUBD_DI)
		return -1;

	if(s->has_insn){
		comedi_insn insn;
		lsampl_t data;
		
		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_READ;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);

		ret = comedi_do_insn(it,&insn);

		*val = data;

		return ret;
	}else{
		comedi_trig trig;
		lsampl_t data;

		memset(&trig,0,sizeof(trig));
		trig.n_chan=1;
		trig.n=1;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=(sampl_t *)&data;

		ret=comedi_trigger(it,&trig);

		if(ret>=0 && val)*val=data;

		return ret;
	}
}

int comedi_dio_write(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int val)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;
	
	s = it->subdevices+subdev;
	if(s->type!=COMEDI_SUBD_DIO &&
	   s->type!=COMEDI_SUBD_DO)
		return -1;

	if(s->has_insn){
		comedi_insn insn;
		lsampl_t data;
		
		memset(&insn,0,sizeof(insn));
		insn.insn = INSN_WRITE;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,0,0);

		data = val;

		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig trig;
		lsampl_t data;

		data=val;

		memset(&trig,0,sizeof(trig));
		trig.n_chan=1;
		trig.n=1;
		trig.flags=TRIG_WRITE;
		trig.subdev=subdev;
		trig.chanlist=&chan;
		trig.data=(sampl_t *)&data;

		return comedi_trigger(it,&trig);
	}
}

int comedi_dio_bitfield(comedi_t *it,unsigned int subdev,unsigned int mask,unsigned int *bits)
{
	int ret;
	unsigned int m,bit;
	subdevice *s;

	if(!valid_subd(it,subdev))
		return -1;

	s=it->subdevices+subdev;

	if(s->type!=COMEDI_SUBD_DIO && s->type!=COMEDI_SUBD_DO &&
	   s->type!=COMEDI_SUBD_DI)
		return -1;

	if(s->has_insn_bits){
		comedi_insn insn;
		lsampl_t data[2];
		
		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_BITS;
		insn.n = 2;
		insn.data = data;
		insn.subdev = subdev;

		data[0]=mask;
		data[1]=*bits;

		ret = comedi_do_insn(it,&insn);

		if(ret<0)return ret;

		*bits = data[1];

		return 0;
	}else{
		unsigned int i,n_chan;

		n_chan=comedi_get_n_channels(it,subdev);
		if(n_chan>32)n_chan=32;
		for(i=0,m=1;i<n_chan;i++,m<<=1){
			if(mask&m){
				bit=(*bits&m)?1:0;
				ret=comedi_dio_write(it,subdev,i,bit);
			}else{
				ret=comedi_dio_read(it,subdev,i,&bit);
				if(bit) *bits|=m;
				else (*bits)&=~m;
			}
			if(ret<0)return ret;
		}
		return (int)n_chan;
	}
}

