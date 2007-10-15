/***********************************************************
           Interface file for wrapping Comedilib

    Copyright (C) 2003 Bryan Cole  <bryan.cole@teraview.co.uk>
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>
    Copyright (C) 2003,2004 Frank Mori Hess <fmhess@users.sourceforge.net>
    Copyright (C) 2003 Steven Jenkins <steven.jenkins@ieee.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
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

/* comedi's major device number */
#define COMEDI_MAJOR 98

/*
   maximum number of minor devices.  This can be increased, although
   kernel structures are currently statically allocated, thus you
   don't want this to be much more than you actually use.
 */
#define COMEDI_NDEVICES 16

/* number of config options in the config structure */
#define COMEDI_NDEVCONFOPTS 32
/*length of nth chunk of firmware data*/
#define COMEDI_DEVCONF_AUX_DATA3_LENGTH		25
#define COMEDI_DEVCONF_AUX_DATA2_LENGTH		26
#define COMEDI_DEVCONF_AUX_DATA1_LENGTH		27
#define COMEDI_DEVCONF_AUX_DATA0_LENGTH		28
#define COMEDI_DEVCONF_AUX_DATA_HI		29	/*most significant 32 bits of pointer address (if needed)*/
#define COMEDI_DEVCONF_AUX_DATA_LO		30	/*least significant 32 bits of pointer address*/
#define COMEDI_DEVCONF_AUX_DATA_LENGTH	31	/* total data length */

/* max length of device and driver names */
#define COMEDI_NAMELEN 20


typedef unsigned int lsampl_t;
typedef unsigned short sampl_t;

/* packs and unpacks a channel/range number */

#define CR_PACK(chan,rng,aref)		( (((aref)&0x3)<<24) | (((rng)&0xff)<<16) | (chan) )
#define CR_PACK_FLAGS(chan, range, aref, flags)	(CR_PACK(chan, range, aref) | ((flags) & CR_FLAGS_MASK))

#define CR_CHAN(a)	((a)&0xffff)
#define CR_RANGE(a)	(((a)>>16)&0xff)
#define CR_AREF(a)	(((a)>>24)&0x03)

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
#define TRIG_WAKE_EOS	0x0020		/* wake up on end-of-scan events */
//#define TRIG_WRITE	0x0040		/* write to bidirectional devices */

/* command flags */
/* These flags are used in comedi_cmd structures */

#define CMDF_PRIORITY		0x00000008 /* try to use a real-time interrupt while performing command */

#define TRIG_RT		CMDF_PRIORITY /* compatibility definition */

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
#define SDF_CMD		0x1000		/* can do commands (deprecated) */
#define SDF_SOFT_CALIBRATED	0x2000	/* subdevice uses software calibration */
#define SDF_CMD_WRITE		0x4000		/* can do output commands */
#define SDF_CMD_READ		0x8000		/* can do input commands */

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

enum comedi_subdevice_type
{
	COMEDI_SUBD_UNUSED,	/* unused by driver */
	COMEDI_SUBD_AI,	/* analog input */
	COMEDI_SUBD_AO,	/* analog output */
	COMEDI_SUBD_DI,	/* digital input */
	COMEDI_SUBD_DO,	/* digital output */
	COMEDI_SUBD_DIO,	/* digital input/output */
	COMEDI_SUBD_COUNTER,	/* counter */
	COMEDI_SUBD_TIMER,	/* timer */
	COMEDI_SUBD_MEMORY,	/* memory, EEPROM, DPRAM */
	COMEDI_SUBD_CALIB,	/* calibration DACs */
	COMEDI_SUBD_PROC,	/* processor, DSP */
	COMEDI_SUBD_SERIAL	/* serial IO */
};

/* configuration instructions */

enum configuration_ids
{
	INSN_CONFIG_DIO_INPUT = 0,
	INSN_CONFIG_DIO_OUTPUT = 1,
	INSN_CONFIG_DIO_OPENDRAIN = 2,
	INSN_CONFIG_ANALOG_TRIG = 16,
//	INSN_CONFIG_WAVEFORM = 17,
//	INSN_CONFIG_TRIG = 18,
//	INSN_CONFIG_COUNTER = 19,
	INSN_CONFIG_ALT_SOURCE = 20,
	INSN_CONFIG_DIGITAL_TRIG = 21,
	INSN_CONFIG_BLOCK_SIZE = 22,
	INSN_CONFIG_TIMER_1 = 23,
	INSN_CONFIG_FILTER = 24,
	INSN_CONFIG_CHANGE_NOTIFY = 25,

	/*ALPHA*/
	INSN_CONFIG_SERIAL_CLOCK = 26,
	INSN_CONFIG_BIDIRECTIONAL_DATA = 27,
	INSN_CONFIG_DIO_QUERY = 28,
	INSN_CONFIG_PWM_OUTPUT = 29,
	INSN_CONFIG_GET_PWM_OUTPUT = 30,
	INSN_CONFIG_ARM = 31,
	INSN_CONFIG_DISARM = 32,
	INSN_CONFIG_GET_COUNTER_STATUS = 33,
	INSN_CONFIG_RESET = 34,
	INSN_CONFIG_GPCT_SINGLE_PULSE_GENERATOR = 1001, // Use CTR as single pulsegenerator
	INSN_CONFIG_GPCT_PULSE_TRAIN_GENERATOR = 1002, // Use CTR as pulsetraingenerator
	INSN_CONFIG_GPCT_QUADRATURE_ENCODER = 1003, // Use the counter as encoder
	INSN_CONFIG_SET_GATE_SRC = 2001,	// Set gate source
	INSN_CONFIG_GET_GATE_SRC = 2002,	// Get gate source
	INSN_CONFIG_SET_CLOCK_SRC = 2003,	// Set master clock source
	INSN_CONFIG_GET_CLOCK_SRC = 2004,	// Get master clock source
	INSN_CONFIG_SET_OTHER_SRC = 2005,       // Set other source
//	INSN_CONFIG_GET_OTHER_SRC = 2006,	// Get other source
	INSN_CONFIG_SET_COUNTER_MODE = 4097,
	INSN_CONFIG_8254_SET_MODE = INSN_CONFIG_SET_COUNTER_MODE,	/* deprecated */
	INSN_CONFIG_8254_READ_STATUS = 4098,
	INSN_CONFIG_SET_ROUTING = 4099,
	INSN_CONFIG_GET_ROUTING = 4109,
};

enum comedi_io_direction
{
	COMEDI_INPUT = 0,
	COMEDI_OUTPUT = 1,
	COMEDI_OPENDRAIN = 2
};

/* ioctls */

#define CIO 'd'
#define COMEDI_DEVCONFIG _IOW(CIO,0,comedi_devconfig)
#define COMEDI_DEVINFO _IOR(CIO,1,comedi_devinfo)
#define COMEDI_SUBDINFO _IOR(CIO,2,comedi_subdinfo)
#define COMEDI_CHANINFO _IOR(CIO,3,comedi_chaninfo)
#define COMEDI_TRIG _IOWR(CIO,4,comedi_trig)
#define COMEDI_LOCK _IO(CIO,5)
#define COMEDI_UNLOCK _IO(CIO,6)
#define COMEDI_CANCEL _IO(CIO,7)
#define COMEDI_RANGEINFO _IOR(CIO,8,comedi_rangeinfo)
#define COMEDI_CMD _IOR(CIO,9,comedi_cmd)
#define COMEDI_CMDTEST _IOR(CIO,10,comedi_cmd)
#define COMEDI_INSNLIST _IOR(CIO,11,comedi_insnlist)
#define COMEDI_INSN _IOR(CIO,12,comedi_insn)
#define COMEDI_BUFCONFIG _IOR(CIO,13,comedi_bufconfig)
#define COMEDI_BUFINFO _IOWR(CIO,14,comedi_bufinfo)
#define COMEDI_POLL _IO(CIO,15)


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

	unsigned int buf_write_ptr;
	unsigned int buf_read_ptr;
	unsigned int buf_write_count;
	unsigned int buf_read_count;

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

/**********************************************************/
/* everything after this line is ALPHA */
/**********************************************************/

/*
  8254 specific configuration.

  It supports two config commands:

  0 ID: INSN_CONFIG_SET_COUNTER_MODE
  1 8254 Mode
    I8254_MODE0, I8254_MODE1, ..., I8254_MODE5
    OR'ed with:
    I8254_BCD, I8254_BINARY

  0 ID: INSN_CONFIG_8254_READ_STATUS
  1 <-- Status byte returned here.
    B7=Output
    B6=NULL Count
    B5-B0 Current mode.

*/

enum i8254_mode
{
	I8254_MODE0 = (0<<1),  /* Interrupt on terminal count */
	I8254_MODE1 = (1<<1),  /* Hardware retriggerable one-shot */
	I8254_MODE2 = (2<<1),  /* Rate generator */
	I8254_MODE3 = (3<<1),  /* Square wave mode */
	I8254_MODE4 = (4<<1),  /* Software triggered strobe */
	I8254_MODE5 = (5<<1),  /* Hardware triggered strobe (retriggerable) */
	I8254_BCD = 1, /* use binary-coded decimal instead of binary (pretty useless) */
	I8254_BINARY = 0
};

/* mode bits for NI general-purpose counters, set with INSN_CONFIG_SET_COUNTER_MODE */
#define NI_GPCT_COUNTING_MODE_SHIFT 16
#define NI_GPCT_INDEX_PHASE_BITSHIFT 20
#define NI_GPCT_COUNTING_DIRECTION_SHIFT 24
enum ni_gpct_mode_bits
{
	NI_GPCT_GATE_ON_BOTH_EDGES_BIT = 0x4,
	NI_GPCT_EDGE_GATE_MODE_MASK = 0x18,
	NI_GPCT_EDGE_GATE_STARTS_STOPS_BITS = 0x0,
	NI_GPCT_EDGE_GATE_STOPS_STARTS_BITS = 0x8,
	NI_GPCT_EDGE_GATE_STARTS_BITS = 0x10,
	NI_GPCT_EDGE_GATE_NO_STARTS_NO_STOPS_BITS = 0x18,
	NI_GPCT_STOP_MODE_MASK = 0x60,
	NI_GPCT_STOP_ON_GATE_BITS = 0x00,
	NI_GPCT_STOP_ON_GATE_OR_TC_BITS = 0x20,
	NI_GPCT_STOP_ON_GATE_OR_SECOND_TC_BITS = 0x40,
	NI_GPCT_LOAD_B_SELECT_BIT = 0x80,
	NI_GPCT_OUTPUT_MODE_MASK = 0x300,
	NI_GPCT_OUTPUT_TC_PULSE_BITS = 0x100,
	NI_GPCT_OUTPUT_TC_TOGGLE_BITS = 0x200,
	NI_GPCT_OUTPUT_TC_OR_GATE_TOGGLE_BITS = 0x300,
	NI_GPCT_HARDWARE_DISARM_MASK = 0xc00,
	NI_GPCT_NO_HARDWARE_DISARM_BITS = 0x000,
	NI_GPCT_DISARM_AT_TC_BITS = 0x400,
	NI_GPCT_DISARM_AT_GATE_BITS = 0x800,
	NI_GPCT_DISARM_AT_TC_OR_GATE_BITS = 0xc00,
	NI_GPCT_LOADING_ON_TC_BIT = 0x1000,
	NI_GPCT_LOADING_ON_GATE_BIT = 0x4000,
	NI_GPCT_COUNTING_MODE_MASK = 0x7 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_NORMAL_BITS = 0x0 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_QUADRATURE_X1_BITS = 0x1 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_QUADRATURE_X2_BITS = 0x2 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_QUADRATURE_X4_BITS = 0x3 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_TWO_PULSE_BITS = 0x4 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_COUNTING_MODE_SYNC_SOURCE_BITS = 0x6 << NI_GPCT_COUNTING_MODE_SHIFT,
	NI_GPCT_INDEX_PHASE_MASK = 0x3 << NI_GPCT_INDEX_PHASE_BITSHIFT,
	NI_GPCT_INDEX_PHASE_LOW_A_LOW_B_BITS = 0x0 << NI_GPCT_INDEX_PHASE_BITSHIFT,
	NI_GPCT_INDEX_PHASE_LOW_A_HIGH_B_BITS = 0x1 << NI_GPCT_INDEX_PHASE_BITSHIFT,
	NI_GPCT_INDEX_PHASE_HIGH_A_LOW_B_BITS = 0x2 << NI_GPCT_INDEX_PHASE_BITSHIFT,
	NI_GPCT_INDEX_PHASE_HIGH_A_HIGH_B_BITS = 0x3 << NI_GPCT_INDEX_PHASE_BITSHIFT,
	NI_GPCT_INDEX_ENABLE_BIT = 0x400000,
	NI_GPCT_COUNTING_DIRECTION_MASK = 0x3 << NI_GPCT_COUNTING_DIRECTION_SHIFT,
	NI_GPCT_COUNTING_DIRECTION_DOWN_BITS = 0x00 << NI_GPCT_COUNTING_DIRECTION_SHIFT,
	NI_GPCT_COUNTING_DIRECTION_UP_BITS = 0x1 << NI_GPCT_COUNTING_DIRECTION_SHIFT,
	NI_GPCT_COUNTING_DIRECTION_HW_UP_DOWN_BITS = 0x2 << NI_GPCT_COUNTING_DIRECTION_SHIFT,
	NI_GPCT_COUNTING_DIRECTION_HW_GATE_BITS = 0x3 << NI_GPCT_COUNTING_DIRECTION_SHIFT,
	NI_GPCT_RELOAD_SOURCE_MASK = 0xc000000,
	NI_GPCT_RELOAD_SOURCE_FIXED_BITS = 0x0,
	NI_GPCT_RELOAD_SOURCE_SWITCHING_BITS = 0x4000000,
	NI_GPCT_RELOAD_SOURCE_GATE_SELECT_BITS = 0x8000000,
	NI_GPCT_OR_GATE_BIT = 0x10000000,
	NI_GPCT_INVERT_OUTPUT_BIT = 0x20000000
};

/* Bits for setting a clock source with
 * INSN_CONFIG_SET_CLOCK_SRC when using NI general-purpose counters. */
enum ni_gpct_clock_source_bits
{
	NI_GPCT_CLOCK_SRC_SELECT_MASK = 0x3f,
	NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS = 0x0,
	NI_GPCT_TIMEBASE_2_CLOCK_SRC_BITS = 0x1,
	NI_GPCT_TIMEBASE_3_CLOCK_SRC_BITS = 0x2,
	NI_GPCT_LOGIC_LOW_CLOCK_SRC_BITS = 0x3,
	NI_GPCT_NEXT_GATE_CLOCK_SRC_BITS = 0x4,
	NI_GPCT_NEXT_TC_CLOCK_SRC_BITS = 0x5,
	NI_GPCT_SOURCE_PIN_i_CLOCK_SRC_BITS = 0x6, /* NI 660x-specific */
	NI_GPCT_PXI10_CLOCK_SRC_BITS = 0x7,
	NI_GPCT_PXI_STAR_TRIGGER_CLOCK_SRC_BITS = 0x8,
	NI_GPCT_ANALOG_TRIGGER_OUT_CLOCK_SRC_BITS = 0x9,
	NI_GPCT_PRESCALE_MODE_CLOCK_SRC_MASK = 0x30000000,
	NI_GPCT_NO_PRESCALE_CLOCK_SRC_BITS = 0x0,
	NI_GPCT_PRESCALE_X2_CLOCK_SRC_BITS = 0x10000000,	/* divide source by 2 */
	NI_GPCT_PRESCALE_X8_CLOCK_SRC_BITS = 0x20000000,	/* divide source by 8 */
	NI_GPCT_INVERT_CLOCK_SRC_BIT = 0x80000000
};

/* Possibilities for setting a gate source with
INSN_CONFIG_SET_GATE_SRC when using NI general-purpose counters.
May be bitwise-or'd with CR_EDGE or CR_INVERT. */
enum ni_gpct_gate_select
{
	/* m-series gates */
	NI_GPCT_TIMESTAMP_MUX_GATE_SELECT = 0x0,
	NI_GPCT_AI_START2_GATE_SELECT = 0x12,
	NI_GPCT_PXI_STAR_TRIGGER_GATE_SELECT = 0x13,
	NI_GPCT_NEXT_OUT_GATE_SELECT = 0x14,
	NI_GPCT_AI_START1_GATE_SELECT = 0x1c,
	NI_GPCT_NEXT_SOURCE_GATE_SELECT = 0x1d,
	NI_GPCT_ANALOG_TRIGGER_OUT_GATE_SELECT = 0x1e,
	NI_GPCT_LOGIC_LOW_GATE_SELECT = 0x1f,
	/* more gates for 660x */
	NI_GPCT_SOURCE_PIN_i_GATE_SELECT = 0x100,
	NI_GPCT_GATE_PIN_i_GATE_SELECT = 0x101,
	/* more gates for 660x "second gate" */
	NI_GPCT_UP_DOWN_PIN_i_GATE_SELECT = 0x201,
	NI_GPCT_SELECTED_GATE_GATE_SELECT = 0x21e,
	/* m-series "second gate" sources are unknown,
	we should add them here with an offset of 0x300 when known. */
	NI_GPCT_DISABLED_GATE_SELECT = 0x8000,
};

/* Possibilities for setting a source with
INSN_CONFIG_SET_OTHER_SRC when using NI general-purpose counters. */
enum ni_gpct_other_index {
  NI_GPCT_SOURCE_ENCODER_A,
  NI_GPCT_SOURCE_ENCODER_B,
  NI_GPCT_SOURCE_ENCODER_Z
};
enum ni_gpct_other_select
{
  	/* m-series gates */
        // Still unknown, probably only need NI_GPCT_PFI_OTHER_SELECT
	NI_GPCT_DISABLED_OTHER_SELECT = 0x8000,
};


/* start sources for ni general-purpose counters for use with
INSN_CONFIG_ARM */
enum ni_gpct_arm_source
{
	NI_GPCT_ARM_IMMEDIATE = 0x0,
	NI_GPCT_ARM_PAIRED_IMMEDIATE = 0x1, /* Start both the counter and the adjacent paired counter simultaneously */
	/* NI doesn't document bits for selecting hardware arm triggers.  If
	the NI_GPCT_ARM_UNKNOWN bit is set, we will pass the least significant
	bits (3 bits for 660x or 5 bits for m-series) through to the hardware.
	This will at least allow someone to figure out what the bits do later.*/
	NI_GPCT_ARM_UNKNOWN = 0x1000,
};

/* digital filtering options for ni 660x for use with INSN_CONFIG_FILTER. */
enum ni_gpct_filter_select
{
	NI_GPCT_FILTER_OFF = 0x0,
	NI_GPCT_FILTER_TIMEBASE_3_SYNC = 0x1,
	NI_GPCT_FILTER_100x_TIMEBASE_1= 0x2,
	NI_GPCT_FILTER_20x_TIMEBASE_1 = 0x3,
	NI_GPCT_FILTER_10x_TIMEBASE_1 = 0x4,
	NI_GPCT_FILTER_2x_TIMEBASE_1 = 0x5,
	NI_GPCT_FILTER_2x_TIMEBASE_3 = 0x6
};

/* PFI digital filtering options for ni m-series for use with INSN_CONFIG_FILTER. */
enum ni_pfi_filter_select
{
	NI_PFI_FILTER_OFF = 0x0,
	NI_PFI_FILTER_125ns = 0x1,
	NI_PFI_FILTER_6425ns = 0x2,
	NI_PFI_FILTER_2550us = 0x3
};

/* master clock sources for ni mio boards and INSN_CONFIG_SET_CLOCK_SRC */
enum ni_mio_clock_source
{
	NI_MIO_INTERNAL_CLOCK = 0,
	NI_MIO_RTSI_CLOCK = 1,	/* doesn't work for m-series, use NI_MIO_PLL_RTSI_CLOCK() */
	/* the NI_MIO_PLL_* sources are m-series only */
	NI_MIO_PLL_PXI_STAR_TRIGGER_CLOCK = 2,
	NI_MIO_PLL_PXI10_CLOCK = 3,
	NI_MIO_PLL_RTSI0_CLOCK = 4
};

/* Signals which can be routed to an NI RTSI pin with INSN_CONFIG_SET_ROUTING.
 The numbers assigned are not arbitrary, they correspond to the bits required
 to program the board. */
enum ni_rtsi_routing
{
	NI_RTSI_OUTPUT_ADR_START1 = 0,
	NI_RTSI_OUTPUT_ADR_START2 = 1,
	NI_RTSI_OUTPUT_SCLKG = 2,
	NI_RTSI_OUTPUT_DACUPDN = 3,
	NI_RTSI_OUTPUT_DA_START1 = 4,
	NI_RTSI_OUTPUT_G_SRC0 = 5,
	NI_RTSI_OUTPUT_G_GATE0 = 6,
	NI_RTSI_OUTPUT_RGOUT0 = 7,
	NI_RTSI_OUTPUT_RTSI_BRD_0 = 8,
	NI_RTSI_OUTPUT_RTSI_OSC = 12 /* pre-m-series always have RTSI clock on line 7 */
};

/* Signals which can be routed to an NI PFI pin on an m-series board
 with INSN_CONFIG_SET_ROUTING.  These numbers are also returned
 by INSN_CONFIG_GET_ROUTING on pre-m-series boards, even though
 their routing cannot be changed.  The numbers assigned are
 not arbitrary, they correspond to the bits required
 to program the board. */
enum ni_pfi_routing
{
	NI_PFI_OUTPUT_PFI_DEFAULT = 0,
	NI_PFI_OUTPUT_AI_START1 = 1,
	NI_PFI_OUTPUT_AI_START2 = 2,
	NI_PFI_OUTPUT_AI_CONVERT = 3,
	NI_PFI_OUTPUT_G_SRC1 = 4,
	NI_PFI_OUTPUT_G_GATE1 = 5,
	NI_PFI_OUTPUT_AO_UPDATE_N = 6,
	NI_PFI_OUTPUT_AO_START1 = 7,
	NI_PFI_OUTPUT_AI_START_PULSE = 8,
	NI_PFI_OUTPUT_G_SRC0 = 9,
	NI_PFI_OUTPUT_G_GATE0 = 10,
	NI_PFI_OUTPUT_EXT_STROBE = 11,
	NI_PFI_OUTPUT_AI_EXT_MUX_CLK = 12,
	NI_PFI_OUTPUT_GOUT0 = 13,
	NI_PFI_OUTPUT_GOUT1 = 14,
	NI_PFI_OUTPUT_FREQ_OUT = 15,
	NI_PFI_OUTPUT_PFI_DO = 16,
	NI_PFI_OUTPUT_I_ATRIG = 17,
	NI_PFI_OUTPUT_RTSI0 = 18,
	NI_PFI_OUTPUT_PXI_STAR_TRIGGER_IN = 26,
	NI_PFI_OUTPUT_SCXI_TRIG1 = 27,
	NI_PFI_OUTPUT_DIO_CHANGE_DETECT_RTSI = 28,
	NI_PFI_OUTPUT_CDI_SAMPLE = 29,
	NI_PFI_OUTPUT_CDO_UPDATE = 30
};

/* NI External Trigger lines.  These values are not arbitrary, but are related to
	the bits required to program the board (offset by 1 for historical reasons). */

/* status bits for INSN_CONFIG_GET_COUNTER_STATUS */
enum comedi_counter_status_flags
{
	COMEDI_COUNTER_ARMED = 0x1,
	COMEDI_COUNTER_COUNTING = 0x2,
	COMEDI_COUNTER_TERMINAL_COUNT = 0x4,
};

/* comedilib.h */

typedef struct comedi_t_struct comedi_t;

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
const char *comedi_get_driver_name(comedi_t *it);
const char *comedi_get_board_name(comedi_t *it);
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
#ifdef _COMEDILIB_DEPRECATED
int comedi_trigger(comedi_t *it,comedi_trig *trig); /* deprecated */
#endif
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
int comedi_dio_get_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *OUTPUT);
int comedi_dio_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *OUTPUT);
int comedi_dio_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int bit);
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *INOUT);
int comedi_dio_bitfield2(comedi_t *it, unsigned int subd,
	unsigned int write_mask, unsigned int *INOUT, unsigned base_channel);

/* slowly varying stuff */
int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan);
int comedi_sv_update(comedi_sv_t *it);
int comedi_sv_measure(comedi_sv_t *it,double *data);

/* streaming I/O (commands) */

int comedi_get_cmd_src_mask(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *INOUT);
int comedi_get_cmd_generic_timed(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *INOUT, unsigned chanlist_len, unsigned int scan_period_ns);
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
int comedi_mark_buffer_written(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_get_buffer_offset(comedi_t *it, unsigned int subdev);

#ifdef _COMEDILIB_DEPRECATED
/*
 * The following functions are deprecated and should not be used.
 */
int comedi_get_timer(comedi_t *it,unsigned int subdev,double freq,
	unsigned int *trigvar,double *actual_freq);
int comedi_timed_1chan(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int range, unsigned int aref,double freq,
	unsigned int n_samples,double *data);
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,
	unsigned int chan);
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *bits);
#endif


#ifndef _COMEDILIB_STRICT_ABI
/*
   The following prototypes are _NOT_ part of the Comedilib ABI, and
   may change in future versions without regard to source or binary
   compatibility.  In practice, this is a holding place for the next
   library ABI version change.
 */
/* structs and functions used for parsing calibration files */
typedef struct
{
	unsigned int subdevice;
	unsigned int channel;
	unsigned int value;
} comedi_caldac_t;
#define COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS 4
typedef struct
{
	double coefficients[COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS];
	double expansion_origin;
	unsigned order;
} comedi_polynomial_t;
typedef struct
{
	comedi_polynomial_t *to_phys;
	comedi_polynomial_t *from_phys;
} comedi_softcal_t;
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
	comedi_softcal_t soft_calibration;
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

/* New stuff to provide conversion between integers and physical values that
* can support software calibrations. */
enum comedi_conversion_direction
{
	COMEDI_TO_PHYSICAL,
	COMEDI_FROM_PHYSICAL
};
int comedi_get_softcal_converter(
	unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction,
	const comedi_calibration_t *calibration, comedi_polynomial_t* OUTPUT);
int comedi_get_hardcal_converter(
	comedi_t *dev, unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction, comedi_polynomial_t* OUTPUT);
double comedi_to_physical(lsampl_t data,
	const comedi_polynomial_t *conversion_polynomial);
lsampl_t comedi_from_physical(double data,
	const comedi_polynomial_t *conversion_polynomial);

#endif
