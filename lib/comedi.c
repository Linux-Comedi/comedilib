/*
    lib/comedi.c
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

int __comedi_init=0;

void initialize(void)
{
	char *s;
	
	__comedi_init=1;

	if( (s=getenv("COMEDILIB_LOGLEVEL")) ){
		__comedi_loglevel=strtol(s,NULL,10);
		fprintf(stderr,"setting loglevel to %d\n",__comedi_loglevel);
	}
}
  
comedi_t *comedi_open(const char *fn)
{
	comedi_t *it;

	if(!__comedi_init)
		initialize();

	if(!(it=malloc(sizeof(comedi_t))))
		goto cleanup;
	memset(it,0,sizeof(comedi_t));

	if((it->fd=open(fn,O_RDWR))<0){
		libc_error();
		goto cleanup;
	}

	if(ioctl_devinfo(it->fd,&it->devinfo)<0)
		goto cleanup;

	it->n_subdevices=it->devinfo.n_subdevs;

	get_subdevices(it);

	it->magic=COMEDILIB_MAGIC;

	return it;
cleanup:
	if(it)
		free(it);

	return NULL;
}

#if 0
/* this is an example of how we do versioned symbols */
__asm__(".symver comedi_open_0,comedi_open@");
#endif

int comedi_close(comedi_t *it)
{
	subdevice *s;
	int i;

	it->magic=0;

	for(i=0;i<it->n_subdevices;i++){
		s=it->subdevices+i;
		if(s->type==COMEDI_SUBD_UNUSED)
			continue;

		if(s->subd_flags&SDF_FLAGS){
			free(s->flags_list);
		}
		if(s->subd_flags&SDF_MAXDATA){
			free(s->maxdata_list);
		}
		if(s->subd_flags&SDF_RANGETYPE){
			free(s->range_type_list);
			free(s->rangeinfo_list);
		}else{
			free(s->rangeinfo);
		}
	}
	if(it->subdevices){
		free(it->subdevices);
	}
	close(it->fd);
	free(it);
	return 0;
}

int comedi_cancel(comedi_t *it,unsigned int subdevice)
{
	return ioctl(it->fd,COMEDI_CANCEL,subdevice);
}

int comedi_poll(comedi_t *it,unsigned int subdevice)
{
	return ioctl(it->fd,COMEDI_POLL,subdevice);
}

int comedi_fileno(comedi_t *it)
{
	if(!it)
		return -1;

	return it->fd;
}

int comedi_trigger(comedi_t *it,comedi_trig *t)
{
	if(!it || !t)
		return -1;

	return ioctl_trigger(it->fd,t);
}

int comedi_command(comedi_t *it,comedi_cmd *t)
{
	int ret;
	ret = ioctl(it->fd,COMEDI_CMD,t);
	__comedi_errno = errno;
	return ret;
}

int comedi_command_test(comedi_t *it,comedi_cmd *t)
{
	int ret;
	ret = ioctl(it->fd,COMEDI_CMDTEST,t);
	__comedi_errno = errno;
	return ret;
}

int comedi_do_insnlist(comedi_t *it,comedi_insnlist *il)
{
	int ret;
	ret = ioctl(it->fd,COMEDI_INSNLIST,il);
	__comedi_errno = errno;
	return ret;
}

int comedi_do_insn(comedi_t *it,comedi_insn *insn)
{
	if(it->has_insn_ioctl){
		return ioctl(it->fd,COMEDI_INSN,insn);
	}else{
		comedi_insnlist il;
		int ret;

		il.n_insns = 1;
		il.insns = insn;

		ret = ioctl(it->fd,COMEDI_INSNLIST,&il);

		if(ret<0)return ret;
		return insn->n;
	}
}

int comedi_lock(comedi_t *it,unsigned int subdevice)
{
	return ioctl(it->fd,COMEDI_LOCK,subdevice);
}

int comedi_unlock(comedi_t *it,unsigned int subdevice)
{
	return ioctl(it->fd,COMEDI_UNLOCK,subdevice);
}

