/*
    lib/comedi.c
    generic functions

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

INTERNAL int __comedi_init=0;

INTERNAL void initialize(void)
{
	char *s;
	
	__comedi_init=1;

	if( (s=getenv("COMEDILIB_LOGLEVEL")) ){
		__comedi_loglevel=strtol(s,NULL,0);
		DEBUG(3,"setting loglevel to %d\n",__comedi_loglevel);
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

	if(comedi_ioctl(it->fd, COMEDI_DEVINFO, (unsigned long)&it->devinfo)<0)
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
		if(s->cmd_mask)free(s->cmd_mask);
		if(s->cmd_timed)free(s->cmd_timed);
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
	return comedi_ioctl(it->fd,COMEDI_CANCEL,subdevice);
}

int comedi_poll(comedi_t *it,unsigned int subdevice)
{
	return comedi_ioctl(it->fd,COMEDI_POLL,subdevice);
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

	return comedi_ioctl(it->fd, COMEDI_TRIG, (unsigned long)t);
}

int comedi_command(comedi_t *it,comedi_cmd *t)
{
	int ret;
	ret = comedi_ioctl(it->fd, COMEDI_CMD, (unsigned long)t);
	__comedi_errno = errno;
	switch(__comedi_errno){
	case EIO:
		__comedi_errno = ECMDNOTSUPP;
		break;
	}
	return ret;
}

int comedi_command_test(comedi_t *it,comedi_cmd *t)
{
	int ret;
	ret = comedi_ioctl(it->fd, COMEDI_CMDTEST, (unsigned long)t);
	__comedi_errno = errno;
	switch(__comedi_errno){
	case EIO:
		__comedi_errno = ECMDNOTSUPP;
		break;
	}
	return ret;
}

int comedi_do_insnlist(comedi_t *it,comedi_insnlist *il)
{
	int ret;
	ret = comedi_ioctl(it->fd, COMEDI_INSNLIST, (unsigned long)il);
	__comedi_errno = errno;
	return ret;
}

int comedi_do_insn(comedi_t *it,comedi_insn *insn)
{
	if(it->has_insn_ioctl){
		return comedi_ioctl(it->fd, COMEDI_INSN, (unsigned long)insn);
	}else{
		comedi_insnlist il;
		int ret;

		il.n_insns = 1;
		il.insns = insn;

		ret = comedi_ioctl(it->fd, COMEDI_INSNLIST, (unsigned long)&il);

		if(ret<0)return ret;
		return insn->n;
	}
}

int comedi_lock(comedi_t *it,unsigned int subdevice)
{
	return comedi_ioctl(it->fd, COMEDI_LOCK, subdevice);
}

int comedi_unlock(comedi_t *it,unsigned int subdevice)
{
	return comedi_ioctl(it->fd, COMEDI_UNLOCK, subdevice);
}

