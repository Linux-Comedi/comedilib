/*
    lib/data.c
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



int comedi_data_write(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,lsampl_t data)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;
	
	if(s->has_insn){
		comedi_insn insn;

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_WRITE;
		insn.n = 1;
		insn.data = &data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,range,aref);

		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig cmd={
			mode:		0,
			flags:		TRIG_WRITE,
			n_chan:		1,
			n:		1,
			trigsrc:	0,
			trigvar:	0,
			trigvar1:	0,
		};
		sampl_t sdata=data;

		chan=CR_PACK(chan,range,aref);

		cmd.subdev=subdev;
		if(it->subdevices[subdev].subd_flags & SDF_LSAMPL){
			cmd.data=(sampl_t *)(&data);
		}else{
			cmd.data=&sdata;
		}
		cmd.chanlist=&chan;

		return ioctl_trigger(it->fd,&cmd);
	}
}


int comedi_data_read(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,lsampl_t *data)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;

	if(s->has_insn){
		comedi_insn insn;

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_READ;
		insn.n = 1;
		insn.data = data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,range,aref);

		return comedi_do_insn(it,&insn);
	}else{
		comedi_trig cmd={
			mode:		0,
			flags:		0,
			n_chan:		1,
			n:		1,
			trigsrc:	0,
			trigvar:	0,
			trigvar1:	0,
		};
		int ret;
		sampl_t sdata;

		chan=CR_PACK(chan,range,aref);
	
		cmd.subdev=subdev;
		cmd.chanlist=&chan;
		if(s->subd_flags & SDF_LSAMPL){
			cmd.data=(sampl_t *)data;
		}else{
			cmd.data=&sdata;
		}

		ret=ioctl_trigger(it->fd,&cmd);
		if(ret<0)
			return ret;

		if(!(s->subd_flags & SDF_LSAMPL)){
			*data=sdata;
		}

		return 0;
	}
}

#if 1
/*
   I don't like this function, which is why it is marked out.
   The problem is the sampl_t/lsampl_t fiasco, which is beginning
   to be a PITA.
 */
int comedi_data_read_n(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,unsigned int n,lsampl_t *data)
{
	comedi_trig cmd;
	unsigned int i;
	int ret;

	if(!valid_chan(it,subdev,chan))
		return -1;
	
	if(n==0)
		return 0;

	chan=CR_PACK(chan,range,aref);
	
	memset(&cmd,0,sizeof(cmd));
	cmd.mode=0;
	cmd.n_chan=1;
	cmd.subdev=subdev;
	cmd.chanlist=&chan;

	i=0;
	while(i<n){
		cmd.data=(void *)(data+i);
		cmd.n=n-i;

		ret=ioctl_trigger(it->fd,&cmd);
		if(ret<0)
			goto out;

		i+=ret;
	}
out:
	if(i==0)return -1;

	return (int)i;
}
#endif



