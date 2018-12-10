/* vim: set ts=8 sw=8 noet tw=80 nowrap: */
/*
    include/comedilib.h
    header file for the comedi library routines

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>

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

*/


#ifndef _COMEDILIB_H
#define _COMEDILIB_H

#include <comedi.h>
#include <comedilib_version.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Macros for swig to %include this file. */
#ifdef SWIG
#define SWIG_OUTPUT(x)  OUTPUT
#define SWIG_INPUT(x)   INPUT
#define SWIG_INOUT(x)   INOUT
#else
#define SWIG_OUTPUT(x)  x
#define SWIG_INPUT(x)   x
#define SWIG_INOUT(x)   x
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

enum comedi_oor_behavior {
	COMEDI_OOR_NUMBER = 0,
	COMEDI_OOR_NAN
};




comedi_t *comedi_open(const char *fn);
int comedi_close(comedi_t *it);

/* logging */
int comedi_loglevel(int loglevel);
void comedi_perror(const char *s);
const char *comedi_strerror(int errnum);
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
/*
 * Changing read and write subdevice is supported for the version of comedi
 * in the Linux kernel sources since Linux kernel version 3.19.
 *
 * The functions to set and get the read and write subdevice are unusual in
 * that changes are local to the "open file description" for the Comedi
 * device.
 */
int comedi_set_read_subdevice(comedi_t *it,unsigned int subdevice);
int comedi_set_write_subdevice(comedi_t *it,unsigned int subdevice);

/* physical units */
double comedi_to_phys(lsampl_t data,comedi_range *rng,lsampl_t maxdata);
lsampl_t comedi_from_phys(double data,comedi_range *rng,lsampl_t maxdata);
int comedi_sampl_to_phys(double *dest, int dst_stride, sampl_t *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);
int comedi_sampl_from_phys(sampl_t *dest,int dst_stride,double *src,
	int src_stride, comedi_range *rng, lsampl_t maxdata, int n);

/* synchronous stuff */
int comedi_data_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *SWIG_OUTPUT(data));
int comedi_data_read_n(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *data, unsigned int n);
int comedi_data_read_hint(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref);
int comedi_data_read_delayed(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *SWIG_OUTPUT(data), unsigned int nano_sec);
int comedi_data_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t data);
int comedi_dio_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int dir);
int comedi_dio_get_config(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *SWIG_OUTPUT(dir));
int comedi_dio_read(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int *SWIG_OUTPUT(bit));
int comedi_dio_write(comedi_t *it,unsigned int subd,unsigned int chan,
	unsigned int bit);
int comedi_dio_bitfield2(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *SWIG_INOUT(bits), unsigned int base_channel);
/* Should be moved to _COMEDILIB_DEPRECATED once bindings for other languages are updated
 * to use comedi_dio_bitfield2() instead.*/
int comedi_dio_bitfield(comedi_t *it,unsigned int subd,
	unsigned int write_mask, unsigned int *SWIG_INOUT(bits));

/* slowly varying stuff */
int comedi_sv_init(comedi_sv_t *it,comedi_t *dev,unsigned int subd,unsigned int chan);
int comedi_sv_update(comedi_sv_t *it);
int comedi_sv_measure(comedi_sv_t *it,double *SWIG_OUTPUT(data));

/* streaming I/O (commands) */

int comedi_get_cmd_src_mask(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *SWIG_INOUT(cmd));
int comedi_get_cmd_generic_timed(comedi_t *dev,unsigned int subdevice,
	comedi_cmd *SWIG_INOUT(cmd), unsigned chanlist_len, unsigned scan_period_ns);
int comedi_cancel(comedi_t *it,unsigned int subdevice);
int comedi_command(comedi_t *it,comedi_cmd *cmd);
int comedi_command_test(comedi_t *it,comedi_cmd *SWIG_INOUT(cmd));
int comedi_poll(comedi_t *dev,unsigned int subdevice);

/* buffer control */

int comedi_set_max_buffer_size(comedi_t *it, unsigned int subdev,
	unsigned int max_size);
int comedi_get_buffer_contents(comedi_t *it, unsigned int subdev);
int comedi_mark_buffer_read(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_mark_buffer_written(comedi_t *it, unsigned int subdev,
	unsigned int bytes);
int comedi_get_buffer_read_offset(comedi_t *it, unsigned int subdev);
int comedi_get_buffer_write_offset(comedi_t *it, unsigned int subdev);
int comedi_get_buffer_read_count(comedi_t *it, unsigned int subdev,
	unsigned int *SWIG_OUTPUT(read_count));
int comedi_get_buffer_write_count(comedi_t *it, unsigned int subdev,
	unsigned int *SWIG_OUTPUT(write_count));

/* Should be moved to _COMEDILIB_DEPRECATED - use comedi_get_buffer_read_offset instead. */
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
	const comedi_calibration_t *calibration, comedi_polynomial_t* SWIG_OUTPUT(polynomial));
int comedi_get_hardcal_converter(
	comedi_t *dev, unsigned subdevice, unsigned channel, unsigned range,
	enum comedi_conversion_direction direction, comedi_polynomial_t* SWIG_OUTPUT(polynomial));
double comedi_to_physical(lsampl_t data,
	const comedi_polynomial_t *conversion_polynomial);
lsampl_t comedi_from_physical(double data,
	const comedi_polynomial_t *conversion_polynomial);

int comedi_internal_trigger(comedi_t *dev, unsigned subd, unsigned trignum);
/* INSN_CONFIG wrappers */
int comedi_arm(comedi_t *device, unsigned subdevice, unsigned source);
int comedi_arm_channel(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned source);
int comedi_disarm(comedi_t *device, unsigned subdevice);
int comedi_disarm_channel(comedi_t *device, unsigned subdevice,
	unsigned channel);
int comedi_reset(comedi_t *device, unsigned subdevice);
int comedi_reset_channel(comedi_t *device, unsigned subdevice,
	unsigned channel);
int comedi_get_clock_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned *SWIG_OUTPUT(clock), unsigned *SWIG_OUTPUT(period_ns));
int comedi_get_gate_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned gate, unsigned *SWIG_OUTPUT(source));
int comedi_get_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned *SWIG_OUTPUT(routing));
int comedi_set_counter_mode(comedi_t *device, unsigned subdevice, unsigned channel, unsigned mode_bits);
int comedi_set_clock_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned clock, unsigned period_ns);
int comedi_set_filter(comedi_t *device, unsigned subdevice, unsigned channel, unsigned filter);
int comedi_set_gate_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned gate_index, unsigned gate_source);
int comedi_set_other_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned other, unsigned source);
int comedi_set_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned routing);
int comedi_get_hardware_buffer_size(comedi_t *device, unsigned subdevice, enum comedi_io_direction direction);
int comedi_digital_trigger_disable(comedi_t *device, unsigned subdevice,
	unsigned trigger_id);
int comedi_digital_trigger_enable_edges(comedi_t *device, unsigned subdevice,
	unsigned trigger_id, unsigned base_input,
	unsigned rising_edge_inputs, unsigned falling_edge_inputs);
int comedi_digital_trigger_enable_levels(comedi_t *device, unsigned subdevice,
	unsigned trigger_id, unsigned base_input,
	unsigned high_level_inputs, unsigned low_level_inputs);

/**
 * Get the hardware timing constraints for a streaming subdevice.
 *
 * The values returned by this function may indicate the range of valid inputs
 * for scan_begin_arg and convert_arg, for instance when *_src==TRIG_TIMER.  For
 * *_src==TRIG_EXT (or other modes?), these returned values are mostly
 * informational and may be used in conjunction with other triggering hardware.
 *
 * If it is possible for the hardware constraints to depend on whether
 * *_src==TRIG_TIMER or *_src==TRIG_EXT, the values returned by this function
 * will depend on these inputs.  Alternatively, one can specify something like
 * *_src==TRIG_TIMER|TRIG_EXT and retrieve the value that is the smallest that
 * satisfies both trigger sources.
 *
 * @param device
 *	Pointer to the device
 * @param subdevice
 *	Index of the streaming/async subdevice to query
 * @param scan_begin_src
 *	Bitwise or of all trigger modes (TRIG_INT, TRIG_EXT, ...) that should be
 *	considered.
 * @param scan_begin_min
 *	If not NULL, this pointer is to return the value of minimum scan_begin
 *	speed that meets the requirements of the slowest trigger specified in
 *	scan_begin_src.
 * @param convert_src
 *	Bitwise or of all trigger modes (TRIG_INT, TRIG_EXT, ...) that should be
 *	considered.
 * @param convert_min
 *	If not NULL, this pointer is to return the value of minimum convert
 *	speed that meets the requirements of the slowest trigger specified in
 *	convert_src.
 * @return 0 if successful or -1 otherwise
 */
int comedi_get_cmd_timing_constraints(comedi_t *device, unsigned int subdevice,
				      unsigned int scan_begin_src,
				      unsigned int *scan_begin_min,
				      unsigned int convert_src,
				      unsigned int *convert_min,
				      unsigned int *chanlist,
				      unsigned int chanlist_len);

/**
 * Validate a globally-named route as connectible and test whether it is
 * connected.
 *
 * Note that since comedi_(dis)connect_route effectively perform the same
 * operations as comedi_{s,g}et_routing and comedi_dio_config, this operation
 * will only report a route as connected _if_ both operations have been
 * performed directly or effectively via comedi_connect_route.
 *
 * There are several routes that are not globally connectable via
 * comedi_connect_route, such as (NI_PFI(i) --> NI_AO_SampleClock) as they must
 * be connected via a trigger argument, such as comedi_cmd->start_arg for
 * NI_AO_StartTrigger.  This function can still be used for such routes to test
 * whether they are valid.
 *
 * @return -1 if not connectible
 * @return 0 if connectible but not connected
 * @return 1 if connectible and connected
 */
int comedi_test_route(comedi_t *device, unsigned source, unsigned destination);

/**
 * Connect a globally-named route.
 *
 * This operation is effectively the same as calling comedi_set_routing and
 * comedi_dio_config on the respective subdevice.
 *
 * There are some routes that are not connectible globally.  For example, the
 * route (NI_PFI(i) --> NI_AO_SampleClock) must actually be specified via the
 * scan_begin_arg argument of a comedi_cmd structure.
 *
 * @return 0 on success
 * @return -1 on error
 *
 * Errors:
 *   EALREADY : The route is already connected.
 *   EINVAL   : The specified route is not valid for this device.
 *   EBUSY    : The destination selector is already busy.
 */
int comedi_connect_route(comedi_t *device, unsigned source, unsigned destination);

/**
 * Disconnect a globally-named route.
 *
 * This operation is effectively the same as calling comedi_set_routing and
 * comedi_dio_config on the respective subdevice.
 *
 * There are some routes that are not dis-connectible globally.  For example,
 * the route (NI_PFI(i) --> NI_AO_SampleClock) must actually be specified via
 * the scan_begin_arg argument of a comedi_cmd structure.
 */
int comedi_disconnect_route(comedi_t *device, unsigned source, unsigned destination);

/**
 * Globally-named route pair to contain values returned by comedi_get_routes.
 * @source:		Globally-identified source of route.
 * @destination:	Globally-identified destination of route.
 */
typedef struct {
	unsigned int source;
	unsigned int destination;
} comedi_route_pair;

/**
 * Obtain a list of all possible globally-named routes for the particular
 * device.
 *
 * A globally-named route is a route where the source and destination values are
 * globally recognized, regardless of the subdevice.
 * @param device
 *	Pointer to the device
 * @param routelist
 *	Array of comedi_route_pair structs that will be used to contain the
 *	returned list.  If this parameter is NULL, comedi_get_routes will return
 *	the number of routes that exist for the device.  Using this feature, one
 *	can query and then allocate routelist appropriately.
 * @param n
 *	The number of elements allocated for in routelist.
 * @return Number of routes available if routelist==NULL
 * @return Number of routes copied if routelist!=NULL
 * @return -1 if there was a problem with the system call
 */
int comedi_get_routes(comedi_t *device, comedi_route_pair * routelist, size_t n);

#endif

#ifdef __cplusplus
}
#endif

#endif

