/*
    lib/ioctl.c
    low-level functions

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


/* ioctl wrappers */

int ioctl_devinfo(int fd,comedi_devinfo *it)
{
	return ioctl(fd,COMEDI_DEVINFO,it);
}

int ioctl_subdinfo(int fd,comedi_subdinfo *it)
{
	return ioctl(fd,COMEDI_SUBDINFO,it);
}

int ioctl_chaninfo(int fd,unsigned int subdev,lsampl_t *maxdata_list,unsigned int *flaglist,unsigned int *rangelist)
{
	comedi_chaninfo ci;

	ci.subdev=subdev;
	ci.flaglist=flaglist;
	ci.rangelist=rangelist;
	ci.maxdata_list=maxdata_list;

	return ioctl(fd,COMEDI_CHANINFO,&ci);
}

int ioctl_trigger(int fd,comedi_trig *it)
{
	return ioctl(fd,COMEDI_TRIG,it);
}

int ioctl_rangeinfo(int fd,int range_type,comedi_krange *range_ptr)
{
	comedi_rangeinfo it;

	it.range_type=range_type;
	it.range_ptr=range_ptr;

	return ioctl(fd,COMEDI_RANGEINFO,&it);
}

int ioctl_bufconfig(int fd, comedi_bufconfig *bc)
{
	return ioctl(fd, COMEDI_BUFCONFIG, bc);
}

int ioctl_bufinfo(int fd, comedi_bufinfo *bi)
{
	return ioctl(fd, COMEDI_BUFINFO, bi);
}
