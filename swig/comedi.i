/***********************************************************
           Interface file for wrapping Comedilib

    Copyright (C) 2003 Bryan Cole  <bryan.cole@teraview.co.uk>
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>
    Copyright (C) 2003,2004 Frank Mori Hess <fmhess@users.sourceforge.net>
    Copyright (C) 2003 Steven Jenkins <steven.jenkins@ieee.org>

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

***********************************************************
*
*     This file was created with Python wrappers in mind but wil
*  probably work for other swig-supported script languages
*
*    to regenerate the wrappers run:
*    swig -python comedi.i
*
***********************************************************/
%module comedi
%{
#include "comedilib.h"
%}
%include "carrays.i"
%include "typemaps.i"

%inline %{
static unsigned int cr_pack(unsigned int chan, unsigned int rng, unsigned int aref){
	return CR_PACK(chan,rng,aref);
}
static unsigned int cr_pack_flags(unsigned int chan, unsigned int rng, unsigned int aref, unsigned int flags){
	return CR_PACK_FLAGS(chan,rng,aref, flags);
}
static unsigned int cr_chan(unsigned int a){
	return CR_CHAN(a);
}
static unsigned int cr_range(unsigned int a){
	return CR_RANGE(a);
}
static unsigned int cr_aref(unsigned int a){
	return CR_AREF(a);
}
%}

%array_class(unsigned int, chanlist);

%typemap(ruby, argout) comedi_cmd *INOUT(VALUE info) {
    $result = output_helper($result, $arg);
};

/* below are (modified) copies of comedi.h and comedilib.h */

/* comedi.h */

typedef unsigned int lsampl_t;
typedef unsigned short sampl_t;

#define CR_FLAGS_MASK	0xfc000000
#define CR_ALT_FILTER	(1<<26)
#define CR_DITHER		CR_ALT_FILTER
#define CR_DEGLITCH		CR_ALT_FILTER
#define CR_ALT_SOURCE	(1<<27)
#define CR_EDGE	(1<<30)
#define CR_INVERT	(1<<31)

#define AREF_GROUND	0x00		/* analog ref = analog ground */
#define AREF_COMMON	0x01		/* analog ref = analog common */
#define AREF_DIFF	0x02		/* analog ref = differential */
#define AREF_OTHER	0x03		/* analog ref = other (undefined) */

/* counters -- these are arbitrary values */
#define GPCT_RESET		0x0001
#define GPCT_SET_SOURCE		0x0002
#define GPCT_SET_GATE		0x0004
#define GPCT_SET_DIRECTION	0x0008
#define GPCT_SET_OPERATION	0x0010
#define GPCT_ARM		0x0020
#define GPCT_DISARM		0x0040
#define GPCT_GET_INT_CLK_FRQ	0x0080

#define GPCT_INT_CLOCK		0x0001
#define GPCT_EXT_PIN		0x0002
#define GPCT_NO_GATE		0x0004
#define GPCT_UP			0x0008
#define GPCT_DOWN		0x0010
#define GPCT_HWUD		0x0020
#define GPCT_SIMPLE_EVENT	0x0040
#define GPCT_SINGLE_PERIOD	0x0080
#define GPCT_SINGLE_PW		0x0100
#define GPCT_CONT_PULSE_OUT	0x0200
#define GPCT_SINGLE_PULSE_OUT	0x0400


/* instructions */

#define INSN_MASK_WRITE		0x8000000
#define INSN_MASK_READ		0x4000000
#define INSN_MASK_SPECIAL	0x2000000

#define INSN_READ		( 0 | INSN_MASK_READ)
#define INSN_WRITE		( 1 | INSN_MASK_WRITE)
#define INSN_BITS		( 2 | INSN_MASK_READ|INSN_MASK_WRITE)
#define INSN_CONFIG		( 3 | INSN_MASK_READ|INSN_MASK_WRITE)
#define INSN_GTOD		( 4 | INSN_MASK_READ|INSN_MASK_SPECIAL)
#define INSN_WAIT		( 5 | INSN_MASK_WRITE|INSN_MASK_SPECIAL)
#define INSN_INTTRIG		( 6 | INSN_MASK_WRITE|INSN_MASK_SPECIAL)

/* trigger flags */
/* These flags are used in comedi_trig structures */

#define TRIG_BOGUS	0x0001		/* do the motions */
#define TRIG_DITHER	0x0002		/* enable dithering */
#define TRIG_DEGLITCH	0x0004		/* enable deglitching */
//#define TRIG_RT	0x0008		/* perform op in real time */
#define TRIG_CONFIG	0x0010		/* perform configuration, not triggering */
//#define TRIG_WAKE_EOS	0x0020		/* wake up on end-of-scan events */
//#define TRIG_WRITE	0x0040		/* write to bidirectional devices */

/* command flags */
/* These flags are used in comedi_cmd structures */

#define CMDF_PRIORITY		0x00000008 /* try to use a real-time interrupt while performing command */

#define TRIG_RT		CMDF_PRIORITY /* compatibility definition */
#define TRIG_WAKE_EOS		0x00000020 /* legacy definition for COMEDI_EV_SCAN_END */

#define CMDF_WRITE		0x00000040
#define TRIG_WRITE	CMDF_WRITE /* compatibility definition */

#define CMDF_RAWDATA		0x00000080

#define COMEDI_EV_START		0x00040000
#define COMEDI_EV_SCAN_BEGIN	0x00080000
#define COMEDI_EV_CONVERT	0x00100000
#define COMEDI_EV_SCAN_END	0x00200000
#define COMEDI_EV_STOP		0x00400000

#define TRIG_ROUND_MASK		0x00030000
#define TRIG_ROUND_NEAREST	0x00000000
#define TRIG_ROUND_DOWN		0x00010000
#define TRIG_ROUND_UP		0x00020000
#define TRIG_ROUND_UP_NEXT	0x00030000

/* trigger sources */

#define TRIG_ANY	0xffffffff
#define TRIG_INVALID	0x00000000

#define TRIG_NONE	0x00000001	/* never trigger */
#define TRIG_NOW	0x00000002	/* trigger now + N ns */
#define TRIG_FOLLOW	0x00000004	/* trigger on next lower level trig */
#define TRIG_TIME	0x00000008	/* trigger at time N ns */
#define TRIG_TIMER	0x00000010	/* trigger at rate N ns */
#define TRIG_COUNT	0x00000020	/* trigger when count reaches N */
#define TRIG_EXT	0x00000040	/* trigger on external signal N */
#define TRIG_INT	0x00000080	/* trigger on comedi-internal signal N */
#define TRIG_OTHER	0x00000100	/* driver defined */

/* subdevice flags */

#define SDF_BUSY	0x0001		/* device is busy */
#define SDF_BUSY_OWNER	0x0002		/* device is busy with your job */
#define SDF_LOCKED	0x0004		/* subdevice is locked */
#define SDF_LOCK_OWNER	0x0008		/* you own lock */
#define SDF_MAXDATA	0x0010		/* maxdata depends on channel */
#define SDF_FLAGS	0x0020		/* flags depend on channel */
#define SDF_RANGETYPE	0x0040		/* range type depends on channel */
#define SDF_MODE0	0x0080		/* can do mode 0 */
#define SDF_MODE1	0x0100		/* can do mode 1 */
#define SDF_MODE2	0x0200		/* can do mode 2 */
#define SDF_MODE3	0x0400		/* can do mode 3 */
#define SDF_MODE4	0x0800		/* can do mode 4 */
#define SDF_CMD		0x1000		/* can do commands */

#define SDF_READABLE	0x00010000	/* subdevice can be read (e.g. analog input) */
#define SDF_WRITABLE	0x00020000	/* subdevice can be written (e.g. analog output) */
#define SDF_WRITEABLE	SDF_WRITABLE	/* spelling error in API */
#define SDF_INTERNAL	0x00040000	/* subdevice does not have externally visible lines */
#define SDF_RT		0x00080000	/* DEPRECATED: subdevice is RT capable */
#define SDF_GROUND	0x00100000	/* can do aref=ground */
#define SDF_COMMON	0x00200000	/* can do aref=common */
#define SDF_DIFF	0x00400000	/* can do aref=diff */
#define SDF_OTHER	0x00800000	/* can do aref=other */
#define SDF_DITHER	0x01000000	/* can do dithering */
#define SDF_DEGLITCH	0x02000000	/* can do deglitching */
#define SDF_MMAP	0x04000000	/* can do mmap() */
#define SDF_RUNNING	0x08000000	/* subdevice is acquiring data */
#define SDF_LSAMPL	0x10000000	/* subdevice uses 32-bit samples */
#define SDF_PACKED	0x20000000	/* subdevice can do packed DIO */

/* subdevice types */

#define COMEDI_SUBD_UNUSED              0	/* unused */
#define COMEDI_SUBD_AI                  1	/* analog input */
#define COMEDI_SUBD_AO                  2	/* analog output */
#define COMEDI_SUBD_DI                  3	/* digital input */
#define COMEDI_SUBD_DO                  4	/* digital output */
#define COMEDI_SUBD_DIO                 5	/* digital input/output */
#define COMEDI_SUBD_COUNTER             6	/* counter */
#define COMEDI_SUBD_TIMER               7	/* timer */
#define COMEDI_SUBD_MEMORY              8	/* memory, EEPROM, DPRAM */
#define COMEDI_SUBD_CALIB               9	/* calibration DACs */
#define COMEDI_SUBD_PROC                10	/* processor, DSP */

/* configuration instructions */

#define COMEDI_INPUT			0
#define COMEDI_OUTPUT			1
#define COMEDI_OPENDRAIN		2

#define INSN_CONFIG_ANALOG_TRIG		0x10
//#define INSN_CONFIG_WAVEFORM		0x11
//#define INSN_CONFIG_TRIG		0x12
//#define INSN_CONFIG_COUNTER		0x13
#define INSN_CONFIG_ALT_SOURCE		0x14
#define INSN_CONFIG_DIGITAL_TRIG	0x15
#define INSN_CONFIG_BLOCK_SIZE		0x16
#define INSN_CONFIG_TIMER_1		0x17
#define INSN_CONFIG_FILTER		0x18
#define INSN_CONFIG_CHANGE_NOTIFY	0x19

/* structures */

typedef struct comedi_trig_struct comedi_trig;
typedef struct comedi_cmd_struct comedi_cmd;
typedef struct comedi_insn_struct comedi_insn;
typedef struct comedi_insnlist_struct comedi_insnlist;
typedef struct comedi_chaninfo_struct comedi_chaninfo;
typedef struct comedi_subdinfo_struct comedi_subdinfo;
typedef struct comedi_devinfo_struct comedi_devinfo;
typedef struct comedi_devconfig_struct comedi_devconfig;
typedef struct comedi_rangeinfo_struct comedi_rangeinfo;
typedef struct comedi_krange_struct comedi_krange;
typedef struct comedi_bufconfig_struct comedi_bufconfig;
typedef struct comedi_bufinfo_struct comedi_bufinfo;

struct comedi_trig_struct{
	unsigned int subdev;		/* subdevice */
	unsigned int mode;		/* mode */
	unsigned int flags;
	unsigned int n_chan;		/* number of channels */
	unsigned int *chanlist; 	/* channel/range list */
	sampl_t *data;			/* data list, size depends on subd flags */
	unsigned int n;			/* number of scans */
	unsigned int trigsrc;
	unsigned int trigvar;
	unsigned int trigvar1;
	unsigned int data_len;
	unsigned int unused[3];
};

struct comedi_insn_struct{
	unsigned int insn;
	unsigned int n;
	lsampl_t *data;
	unsigned int subdev;
	unsigned int chanspec;
	unsigned int unused[3];
};

struct comedi_insnlist_struct{
	unsigned int n_insns;
	comedi_insn *insns;
};

struct comedi_cmd_struct{
	unsigned int subdev;
	unsigned int flags;

	unsigned int start_src;
	unsigned int start_arg;

	unsigned int scan_begin_src;
	unsigned int scan_begin_arg;

	unsigned int convert_src;
	unsigned int convert_arg;

	unsigned int scan_end_src;
	unsigned int scan_end_arg;

	unsigned int stop_src;
	unsigned int stop_arg;

	unsigned int *chanlist; 	/* channel/range list */
	unsigned int chanlist_len;

	sampl_t *data;			/* data list, size depends on subd flags */
	unsigned int data_len;
};

struct comedi_chaninfo_struct{
	unsigned int subdev;
	lsampl_t *maxdata_list;
	unsigned int *flaglist;
	unsigned int *rangelist;
	unsigned int unused[4];
};

struct comedi_rangeinfo_struct{
	unsigned int range_type;
	void *range_ptr;
};

struct comedi_krange_struct{
	int min;	/* fixed point, multiply by 1e-6 */
	int max;	/* fixed point, multiply by 1e-6 */
	unsigned int flags;
};

struct comedi_subdinfo_struct{
	unsigned int type;
	unsigned int n_chan;
	unsigned int subd_flags;
	unsigned int timer_type;
	unsigned int len_chanlist;
	lsampl_t	maxdata;
	unsigned int	flags;		/* channel flags */
	unsigned int	range_type;	/* lookup in kernel */
	unsigned int	settling_time_0;
	unsigned int unused[9];
};

struct comedi_devinfo_struct{
	unsigned int version_code;
	unsigned int n_subdevs;
	char driver_name[COMEDI_NAMELEN];
	char board_name[COMEDI_NAMELEN];
	int read_subdevice;
	int write_subdevice;
	int unused[30];
};

struct comedi_devconfig_struct{
	char board_name[COMEDI_NAMELEN];
	int options[COMEDI_NDEVCONFOPTS];
};

struct comedi_bufconfig_struct{
	unsigned int subdevice;
	unsigned int flags;

	unsigned int maximum_size;
	unsigned int size;

	unsigned int unused[4];
};

struct comedi_bufinfo_struct{
	unsigned int subdevice;
	unsigned int bytes_read;

	unsigned int buf_int_ptr;
	unsigned int buf_user_ptr;
	unsigned int buf_int_count;
	unsigned int buf_user_count;

	unsigned int bytes_written;

	unsigned int unused[4];
};

/* range stuff */

#define __RANGE(a,b)	((((a)&0xffff)<<16)|((b)&0xffff))

#define RANGE_OFFSET(a)		(((a)>>16)&0xffff)
#define RANGE_LENGTH(b)		((b)&0xffff)

#define RF_UNIT(flags)		((flags)&0xff)
#define RF_EXTERNAL		(1<<8)

#define UNIT_volt		0
#define UNIT_mA			1
#define UNIT_none		2

#define COMEDI_MIN_SPEED	((unsigned int)0xffffffff)

/* callback stuff */
/* only relevant to kernel modules. */

#define COMEDI_CB_EOS		1	/* end of scan */
#define COMEDI_CB_EOA		2	/* end of acquisition */
#define COMEDI_CB_BLOCK		4	/* DEPRECATED: convenient block size */
#define COMEDI_CB_EOBUF		8	/* DEPRECATED: end of buffer */
#define COMEDI_CB_ERROR		16	/* card error during acquisition */
#define COMEDI_CB_OVERFLOW	32	/* buffer overflow/underflow */

/* comedilib.h */
typedef void comedi_t;

typedef struct{
	double min;
	double max;
	unsigned int unit;
}comedi_range;

typedef struct comedi_sv_struct{
	comedi_t *dev;
	unsigned int subdevice;
	unsigned int chan;

	/* range policy */
	int range;
	int aref;

	/* number of measurements to average (for ai) */
	int n;

	lsampl_t maxdata;
}comedi_sv_t;

enum comedi_oor_behavior {
	COMEDI_OOR_NUMBER = 0,
	COMEDI_OOR_NAN
};

comedi_t *comedi_open(const char *fn);
int comedi_close(comedi_t *it);

/* logging */
int comedi_loglevel(int loglevel);
void comedi_perror(const char *s);
char *comedi_strerror(int errnum);
int comedi_errno(void);
int comedi_fileno(comedi_t *it);

/* global behavior */
enum comedi_oor_behavior comedi_set_global_oor_behavior(enum comedi_oor_behavior behavior);

/* device queries */
int comedi_get_n_subdevices(comedi_t *it);
#define COMEDI_VERSION_CODE(a,b,c) (((a)<<16) | ((b)<<8) | (c))
int comedi_get_version_code(comedi_t *it);
char *comedi_get_driver_name(comedi_t *it);
char *comedi_get_board_name(comedi_t *it);
int comedi_get_read_subdevice(comedi_t *dev);
int comedi_get_write_subdevice(comedi_t *dev);

/* subdevice queries */
int comedi_get_subdevice_type(comedi_t *it,unsigned int subdevice);
int comedi_find_subdevice_by_type(comedi_t *it,int type,unsigned int subd);
int comedi_get_subdevice_flags(comedi_t *it,unsigned int subdevice);
int comedi_get_n_channels(comedi_t *it,unsigned int subdevice);
int comedi_range_is_chan_specific(comedi_t *it,unsigned int subdevice);
int comedi_maxdata_is_chan_specific(comedi_t *it,unsigned int subdevice);

/* channel queries */
lsampl_t comedi_get_maxdata(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
int comedi_get_n_ranges(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
comedi_range * comedi_get_range(comedi_t *it,unsigned int subdevice,
	unsigned int chan,unsigned int range);
int comedi_find_range(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int unit,double min,double max);

/* buffer queries */
int comedi_get_buffer_size(comedi_t *it,unsigned int subdevice);
int comedi_get_max_buffer_size(comedi_t *it,unsigned int subdevice);
int comedi_set_buffer_size(comedi_t *it,unsigned int subdevice,
	unsigned int len);

/* low-level stuff */
int comedi_do_insnlist(comedi_t *it,comedi_insnlist *il);
int comedi_do_insn(comedi_t *it,comedi_insn *insn);
int comedi_lock(comedi_t *it,unsigned int subdevice);
int comedi_unlock(comedi_t *it,unsigned int subdevice);

/* physical units */
double comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata);
lsampl_t comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata);
int comedi_sampl_to_phys(double *dest, int dst_stride, sampl_t *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);
int comedi_sampl_from_phys(sampl_t *dest,int dst_stride,double *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);

/* syncronous stuff */
int comedi_data_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *OUTPUT);
int comedi_data_read_n(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *OUTPUT, unsigned int n);
int comedi_data_read_hint(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref);
int comedi_data_read_delayed(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *OUTPUT, unsigned int nano_sec);
int comedi_data_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t data);
int comedi_dio_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int dir);
int comedi_dio_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *OUTPUT);
int comedi_dio_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int bit);
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *INOUT);

/* slowly varying stuff */
int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan);
int comedi_sv_update(comedi_sv_t *it);
int comedi_sv_measure(comedi_sv_t *it,double *data);

/* streaming I/O (commands) */

int comedi_get_cmd_src_mask(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *INOUT);
int comedi_get_cmd_generic_timed(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *INOUT,unsigned int ns);
int comedi_cancel(comedi_t *it,unsigned int subdevice);
int comedi_command(comedi_t *it,comedi_cmd *cmd);
int comedi_command_test(comedi_t *it,comedi_cmd *INOUT);
int comedi_poll(comedi_t *dev,unsigned int subdevice);

/* buffer control */

int comedi_set_max_buffer_size(comedi_t *it, unsigned int subdev,
	unsigned int max_size);
int comedi_get_buffer_contents(comedi_t *it, unsigned int subdev);
int comedi_mark_buffer_read(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_get_buffer_offset(comedi_t *it, unsigned int subdev);


/* structs and functions used for parsing calibration files */
typedef struct
{
	unsigned int subdevice;
	unsigned int channel;
	unsigned int value;
} comedi_caldac_t;
#define CS_MAX_AREFS_LENGTH 4
typedef struct
{
	unsigned int subdevice;
	unsigned int *channels;
	unsigned int num_channels;
	unsigned int *ranges;
	unsigned int num_ranges;
	unsigned int arefs[ CS_MAX_AREFS_LENGTH ];
	unsigned int num_arefs;
	comedi_caldac_t *caldacs;
	unsigned int num_caldacs;
} comedi_calibration_setting_t;

typedef struct
{
	char *driver_name;
	char *board_name;
	comedi_calibration_setting_t *settings;
	unsigned int num_settings;
} comedi_calibration_t;

comedi_calibration_t* comedi_parse_calibration_file( const char *cal_file_path );
int comedi_apply_parsed_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const comedi_calibration_t *calibration );
char* comedi_get_default_calibration_path( comedi_t *dev );
void comedi_cleanup_calibration( comedi_calibration_t *calibration );
int comedi_apply_calibration( comedi_t *dev, unsigned int subdev, unsigned int channel,
	unsigned int range, unsigned int aref, const char *cal_file_path);

