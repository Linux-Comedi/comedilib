/*
    include/comedilib.h
    header file for the comedi library routines

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


#ifndef _COMEDILIB_H
#define _COMEDILIB_H

#include <comedi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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




comedi_t *comedi_open(const char *fn);
void comedi_close(comedi_t *it);

int comedi_loglevel(int loglevel);
void comedi_perror(const char *s);
char *comedi_strerror(int errnum);
int comedi_errno(void);
int comedi_fileno(comedi_t *it);

/* queries */

int comedi_get_n_subdevices(comedi_t *it);
int comedi_get_version_code(comedi_t *it);
char *comedi_get_driver_name(comedi_t *it);
char *comedi_get_board_name(comedi_t *it);

int comedi_get_subdevice_type(comedi_t *it,unsigned int subdevice);
int comedi_find_subdevice_by_type(comedi_t *it,int type,unsigned int subd);
int comedi_get_n_channels(comedi_t *it,unsigned int subdevice);
lsampl_t comedi_get_maxdata(comedi_t *it,unsigned int subdevice,unsigned int chan);
int comedi_get_rangetype(comedi_t *it,unsigned int subdevice,unsigned int chan);
comedi_range * comedi_get_range(comedi_t *it,unsigned int subdevice,unsigned int chan,unsigned int range);
int comedi_find_range(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int unit,double min,double max);
int comedi_get_n_ranges(comedi_t *it,unsigned int subdevice,unsigned int chan);

int comedi_range_is_chan_specific(comedi_t *it,unsigned int subdevice);
int comedi_maxdata_is_chan_specific(comedi_t *it,unsigned int subdevice);

/* triggers and commands */

int comedi_cancel(comedi_t *it,unsigned int subdevice);
int comedi_trigger(comedi_t *it,comedi_trig *trig);
int comedi_command(comedi_t *it,comedi_cmd *cmd);
int comedi_command_test(comedi_t *it,comedi_cmd *cmd);
int comedi_do_insnlist(comedi_t *it,comedi_insnlist *il);
int comedi_do_insn(comedi_t *it,comedi_insn *insn);
int comedi_lock(comedi_t *it,unsigned int subdevice);
int comedi_unlock(comedi_t *it,unsigned int subdevice);

/* physical units */

double comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata);
lsampl_t comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata);

/* synchronous stuff */

int comedi_data_read(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int range,
	unsigned int aref,lsampl_t *data);
int comedi_data_write(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int range,
	unsigned int aref,lsampl_t data);

/* slowly varying stuff */

int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan);
int comedi_sv_update(comedi_sv_t *it);
int comedi_sv_measure(comedi_sv_t *it,double *data);

/* dio config */

int comedi_dio_config(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int dir);
int comedi_dio_read(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int *bit);
int comedi_dio_write(comedi_t *it,unsigned int subd,unsigned int chan,unsigned int bit);
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,unsigned int write_mask,
	unsigned int *bits);

/* timer stuff */

int comedi_get_timer(comedi_t *it,unsigned int subdev,double freq,unsigned int *trigvar,
	double *actual_freq);

int comedi_timed_1chan(comedi_t *it,unsigned int subdev,unsigned int chan,unsigned int range,
	unsigned int aref,double freq,unsigned int n_samples,double *data);


/*
   The following prototypes are ALPHA, indicating that they might change
   in future releases of comedilib, without regard to backward compatibility.
 */

enum comedi_oor_behavior {
	COMEDI_OOR_NUMBER = 0,
	COMEDI_OOR_NAN
};

enum comedi_oor_behavior comedi_set_global_oor_behavior(enum comedi_oor_behavior behavior);




#ifdef __cplusplus
}
#endif

#endif

