/*
    lib/libinternal.h
    internal definitions for comedilib

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

#ifndef _LIBINTERNAL_H
#define _LIBINTERNAL_H

#define _COMEDILIB_DEPRECATED

#include <comedilib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef I18N
#include <libintl.h>
#endif

#include <comedi_errno.h>

/* This indicates a symbol that should not be exported as part of
 * the library.  But I don't know how to make it useful yet. */
#define INTERNAL

/* gettext()ization */

#ifdef I18N
#define GETTEXT(a) gettext((a))
#else
#define GETTEXT(a) (a)
#endif
#define _s(a) (a)


#define debug_ptr(a)    if(!(a))fprintf(stderr," ** NULL pointer: " __FILE__ ", line %d\n",__LINE__);
#define debug_int(a)    if((a)<0)fprintf(stderr," ** error: " __FILE__ ", line %d\n",__LINE__);

#define COMEDILIB_DEBUG(level,format,args...) do{if(__comedi_loglevel>=(level))fprintf(stderr,__FUNCTION__ ": " format, ## args);}while(0)

#define COMEDI_VERSION_CODE(a,b,c) (((a)<<16) | ((b)<<8) | (c))

#define COMEDILIB_MAGIC 0xc001dafe

/* handle versioning */

#define EXPORT_SYMBOL(a,b) __asm__(".symver " #a "," #a "@v" #b )
#define EXPORT_ALIAS_VER(a,b,c) __asm__(".symver " #a "," #b "@v" #c )
#define EXPORT_ALIAS_DEFAULT(a,b,c) __asm__(".symver " #a "," #b "@@v" #c )


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

	int cmd_mask_errno;
	comedi_cmd *cmd_mask;
	int cmd_timed_errno;
	comedi_cmd *cmd_timed;
};

#define comedi_ioctl _comedi_ioctl
//#define comedi_ioctl _comedi_ioctl_debug

int _comedi_ioctl( int fd, int request, unsigned long arg );
int _comedi_ioctl_debug(int,int,unsigned long);

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

/* used by range.c, was in comedilib.h but apparently deprecated so I put it here - fmhess */
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,unsigned int chan);

/* structs and functions used for parsing calibration files */
struct caldac_setting
{
	unsigned int subdevice;
	unsigned int channel;
	unsigned int value;
};

struct calibration_setting
{
	unsigned int subdevice;
	unsigned int *channels;
	unsigned int num_channels;
	unsigned int *ranges;
	unsigned int num_ranges;
	unsigned int arefs[ 4 ];
	unsigned int num_arefs;
	struct caldac_setting *caldacs;
	unsigned int num_caldacs;
};

struct calibration_file_contents
{
	char *driver_name;
	char *board_name;
	struct calibration_setting *calibrations;
	unsigned int num_calibrations;
};

int calib_yylex( void );
void calib_yyerror( char *s );
int calib_yyparse( void );
const struct calibration_file_contents* parse_calibration_file( FILE *file );
void cleanup_calibration_parse( void );

#endif

