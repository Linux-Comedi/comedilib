/*
    lib/error.c
    error handling routines

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
#include <string.h>
#include <comedi_errno.h>

char *__comedilib_error_strings[]={
	"No error",
	"Unknown error",
	"Bad comedi_t structure",
	"Invalid subdevice",
	"Invalid channel",
	"Buffer overflow",
	"Buffer underflow",
	"Command not supported",
	"Not supported",
};
#define n_errors (sizeof(__comedilib_error_strings)/sizeof(void *))

int __comedi_loglevel=1;
int __comedi_errno=0;

int comedi_loglevel(int loglevel)
{
	int old_loglevel=__comedi_loglevel;
	
	__comedi_loglevel=loglevel;
	
	return old_loglevel;
}

int comedi_errno(void)
{
	return __comedi_errno;
}

char *comedi_strerror(int errnum)
{
	if(errnum<COMEDI_NOERROR || errnum>=COMEDI_NOERROR+n_errors)
		return strerror(errnum);

	return __comedilib_error_strings[errnum-COMEDI_NOERROR];
}

void comedi_perror(const char *s)
{
	if(__comedi_loglevel>=3){
		fprintf(stderr,"comedi_perror(): __comedi_errno=%d\n",__comedi_errno);
	}
	if(!s)s="comedilib";
	fprintf(stderr,"%s: %s\n",s,comedi_strerror(__comedi_errno));
}

void libc_error(void)
{
	__comedi_errno=errno;
	if(__comedi_loglevel>=2){
		comedi_perror("libc error");
	}
}

void internal_error(int err)
{
	__comedi_errno=err;
	if(__comedi_loglevel>=2){
		comedi_perror("internal error");
	}
}



int valid_dev(comedi_t *it)
{
	if(!it || it->magic!=COMEDILIB_MAGIC){
		internal_error(COMEDILIB_BADDEV);
		return 0;
	}
	
	return 1;
}

int valid_subd(comedi_t *it,unsigned int subd)
{
	if(!valid_dev(it))return 0;
	if(subd>=it->n_subdevices){
		internal_error(COMEDILIB_BADSUBD);
		return 0;
	}
	
	return 1;
}

int valid_chan(comedi_t *it,unsigned int subd,unsigned int chan)
{
	if(!valid_subd(it,subd))return 0;
	if(chan>=it->subdevices[subd].n_chan){
		internal_error(COMEDILIB_BADCHAN);
		return 0;
	}
	
	return 1;
}


