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

static int do_test_for_cmd(comedi_t *dev,unsigned int subdevice);
static int do_test_for_insn(comedi_t *dev);
static int do_test_for_insnlist(comedi_t *dev);
static int do_test_for_insn_bits(comedi_t *dev,unsigned int subdevice);


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

	it->has_insnlist_ioctl = do_test_for_insnlist(it);
	it->has_insn_ioctl = do_test_for_insn(it);
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

		r[i].has_cmd = do_test_for_cmd(it,i);
		if(it->has_insnlist_ioctl){
			r[i].has_insn_bits = do_test_for_insn_bits(it,i);
		}else{
			r[i].has_insn_bits = 0;
		}
	}

	free(s);

	return 0;
}

comedi_range *get_rangeinfo(int fd,unsigned int range_type)
{
	comedi_krange *kr;
	comedi_range *r;
	int ret;
	int i;

	kr=malloc(sizeof(comedi_krange)*RANGE_LENGTH(range_type));
	r=malloc(sizeof(comedi_range)*RANGE_LENGTH(range_type));

	ret=ioctl_rangeinfo(fd,range_type,kr);
	if(ret<0){
		fprintf(stderr,"ioctl_rangeinfo(%d,0x%08x,%p)\n",fd,range_type,kr);
	}

	for(i=0;i<RANGE_LENGTH(range_type);i++){
		r[i].min=kr[i].min*1e-6;
		r[i].max=kr[i].max*1e-6;
		r[i].unit=kr[i].flags;
	}
	free(kr);

	return r;
}


/* some command testing */

static int do_test_for_cmd(comedi_t *dev,unsigned int subdevice)
{
	comedi_cmd it;
	int ret;

	memset(&it,0,sizeof(it));

	it.subdev = 0;
	it.start_src = TRIG_ANY;
	it.scan_begin_src = TRIG_ANY;
	it.convert_src = TRIG_ANY;
	it.scan_end_src = TRIG_ANY;
	it.stop_src = TRIG_ANY;

	ret = ioctl(dev->fd,COMEDI_CMDTEST,&it);

	if(ret<0 && errno==EIO){
		return 0;
	}
	if(ret<0){
		fprintf(stderr,"BUG in do_test_for_cmd()\n");
		return 0;
	}
	return 1;
}

static int do_test_for_insnlist(comedi_t *dev)
{
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data[2];
	int ret;

	memset(&insn,0,sizeof(insn));

	il.n_insns = 1;
	il.insns = &insn;

	insn.insn = INSN_GTOD;
	insn.n = 2;
	insn.data = data;

	ret = ioctl(dev->fd,COMEDI_INSNLIST,&il);

	if(ret<0){
		if(errno!=EIO){
			fprintf(stderr,"BUG in do_test_for_insn()\n");
		}
		return 0;
	}
	return 1;
}

/* the COMEID_INSN ioctl was introduced in comedi-0.7.60 */
static int do_test_for_insn(comedi_t *dev)
{
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data[2];
	int ret;

	memset(&insn,0,sizeof(insn));

	il.n_insns = 1;
	il.insns = &insn;

	insn.insn = INSN_GTOD;
	insn.n = 2;
	insn.data = data;

	ret = ioctl(dev->fd,COMEDI_INSN,&insn);

	if(ret<0){
		if(errno!=EIO){
			fprintf(stderr,"BUG in do_test_for_insn()\n");
		}
		return 0;
	}
	return 1;
}

static int do_test_for_insn_bits(comedi_t *dev,unsigned int subdevice)
{
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data[2];
	int ret;

	memset(&insn,0,sizeof(insn));

	il.n_insns = 1;
	il.insns = &insn;

	insn.insn = INSN_BITS;
	insn.n = 2;
	insn.data = data;
	insn.subdev = subdevice;

	data[0]=0;
	data[1]=0;

	ret = comedi_do_insnlist(dev,&il);

	if(ret<0 && (errno==EINVAL || errno==EIO)){
		return 0;
	}
	if(ret<0){
		fprintf(stderr,"BUG in do_test_for_insn_bits()\n");
		return 0;
	}
	return 1;
}



