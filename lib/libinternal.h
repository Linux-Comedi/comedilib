/*
    lib/libinternal.h
    header file for comedilib internals

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998 David A. Schleef <ds@stm.lbl.gov>

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


#ifndef _LIBINTERNAL_H
#define _LIBINTERNAL_H

#define _COMEDILIB_DEPRECATED

#include <comedilib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>


#define debug_ptr(a)    if(!(a))fprintf(stderr," ** NULL pointer: " __FILE__ ", line %d\n",__LINE__);
#define debug_int(a)    if((a)<0)fprintf(stderr," ** error: " __FILE__ ", line %d\n",__LINE__);

#define COMEDILIB_MAGIC 0xc001dafe


extern int __comedi_init;
extern int __comedi_loglevel;
extern int __comedi_errno;

#if 0

#define libc_error()		(__comedi_errno=errno)
#define internal_error(a)	(__comedi_errno=(a))

#else

void libc_error(void);
void internal_error(int error_number);

#endif


typedef struct subdevice_struct subdevice;
typedef struct device_struct device;

struct comedi_t_struct{
	int magic;

	int fd;
	int n_subdevices;

	comedi_devinfo devinfo;

	subdevice *subdevices;

	unsigned int has_insnlist_ioctl;
	unsigned int has_insn_ioctl;
};

struct subdevice_struct{
	unsigned int type;
	unsigned int n_chan;
	unsigned int subd_flags;
	unsigned int timer_type;
	unsigned int len_chanlist;
	lsampl_t maxdata;
	unsigned int flags;
	unsigned int range_type;

	lsampl_t *maxdata_list;
	unsigned int *range_type_list;
	unsigned int *flags_list;

	comedi_range *rangeinfo;
	comedi_range **rangeinfo_list;

	unsigned int has_cmd;
	unsigned int has_insn_bits;
};



/* ioctl wrappers */

int ioctl_devinfo(int fd,comedi_devinfo *it);
int ioctl_subdinfo(int fd,comedi_subdinfo *it);
int ioctl_chaninfo(int fd,unsigned int subdev,lsampl_t *maxdata_list,
		                unsigned int *flaglist,unsigned int *rangelist);
int ioctl_trigger(int fd,comedi_trig *it);
int ioctl_rangeinfo(int fd,int range_type,comedi_krange *range_ptr);
int ioctl_bufconfig(int fd, comedi_bufconfig *bc);
int ioctl_bufinfo(int fd, comedi_bufinfo *bi);

/* filler routines */

int get_subdevices(comedi_t *it);
comedi_range *get_rangeinfo(int fd,unsigned int range_type);

/* validators */

int valid_dev(comedi_t *it);
int valid_subd(comedi_t *it,unsigned int subdevice);
int valid_chan(comedi_t *it,unsigned int subdevice,unsigned int chan);

enum{
	COMEDILIB_NOERROR = 0x1000,
	COMEDILIB_UNKNOWN,
	COMEDILIB_BADDEV,
	COMEDILIB_BADSUBD,
	COMEDILIB_BADCHAN,
};

// used by range.c, was in comedilib.h but apparently deprecated so I put it here - fmhess
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,unsigned int chan);

#endif

