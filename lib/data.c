/*
    lib/data.c
    functions for reading/writing data

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



int comedi_data_write(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,lsampl_t data)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;
	
	if(it->has_insnlist_ioctl){
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


int comedi_data_read_n(comedi_t *it,unsigned int subdev,unsigned int chan,
		unsigned int range, unsigned int aref,lsampl_t *data,
		unsigned int n)
{
	subdevice *s;
	comedi_insn insn;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;

	if(it->has_insnlist_ioctl){

		memset(&insn,0,sizeof(insn));

		insn.insn = INSN_READ;
		insn.n = 1;
		insn.data = data;
		insn.subdev = subdev;
		insn.chanspec = CR_PACK(chan,range,aref);

		return comedi_do_insn(it,&insn);
	}else{
		/* There's no need to be fast for a case that is
		 * obsolete. */
		return comedi_data_read(it,subdev,chan,range,aref,data);
	}
}

int comedi_data_read(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
		unsigned int aref,lsampl_t *data)
{
	subdevice *s;

	if(!valid_chan(it,subdev,chan))
		return -1;

	s=it->subdevices+subdev;

	if(it->has_insnlist_ioctl){
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

