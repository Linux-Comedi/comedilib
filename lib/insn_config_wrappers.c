/* vim: set ts=8 sw=8 noet tw=80 nowrap: */
/*
    lib/insn_config_wrappers.c
    wrappers for various INSN_CONFIG instructions

    COMEDILIB - Linux Control and Measurement Device Interface Library
    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>
    Copyright (C) 2008 Frank Mori Hess <fmhess@users.sourceforge.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
*/

#include <string.h>
#include <stdlib.h>

#include "libinternal.h"

EXPORT_ALIAS_DEFAULT(_comedi_reset_channel,comedi_reset_channel,0.11.0);
int _comedi_reset_channel(comedi_t *device, unsigned subdevice, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_RESET;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_reset,comedi_reset,0.9.0);
int _comedi_reset(comedi_t *device, unsigned subdevice)
{
	return _comedi_reset_channel(device, subdevice, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_arm_channel,comedi_arm_channel,0.11.0);
int _comedi_arm_channel(comedi_t *device, unsigned subdevice, unsigned channel, unsigned target)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_ARM;
	data[1] = target;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_arm,comedi_arm,0.9.0);
int _comedi_arm(comedi_t *device, unsigned subdevice, unsigned target)
{
	return _comedi_arm_channel(device, subdevice, 0, target);
}

EXPORT_ALIAS_DEFAULT(_comedi_disarm_channel,comedi_disarm_channel,0.11.0);
int _comedi_disarm_channel(comedi_t *device, unsigned subdevice, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_DISARM;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_disarm,comedi_disarm,0.11.0);
int _comedi_disarm(comedi_t *device, unsigned subdevice)
{
	return _comedi_disarm_channel(device, subdevice, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_get_clock_source,comedi_get_clock_source,0.10.0);
int _comedi_get_clock_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned *clock, unsigned *period_ns)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	memset(data, 0, insn.n * sizeof(data[0]));
	data[0] = INSN_CONFIG_GET_CLOCK_SRC;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0) return -1;
	if(clock) *clock = insn.data[1];
	if(period_ns) *period_ns = insn.data[2];
	return 0;
}

EXPORT_ALIAS_VER(_comedi_get_clock_source_chan0,comedi_get_clock_source,0.9.0);
int _comedi_get_clock_source_chan0(comedi_t *device, unsigned subdevice, unsigned *clock, unsigned *period_ns)
{
	return _comedi_get_clock_source(device, subdevice, 0, clock, period_ns);
}

EXPORT_ALIAS_DEFAULT(_comedi_get_gate_source,comedi_get_gate_source,0.9.0);
int _comedi_get_gate_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned gate, unsigned *source)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	memset(insn.data, 0, insn.n * sizeof(insn.data[0]));
	data[0] = INSN_CONFIG_GET_GATE_SRC;
	data[1] = gate;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0) return -1;
	if(source) *source = insn.data[2];
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_routing,comedi_get_routing,0.9.0);
int _comedi_get_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned *routing)
{
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	memset(insn.data, 0, insn.n * sizeof(insn.data[0]));
	data[0] = INSN_CONFIG_GET_ROUTING;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0) return -1;
	if(routing) *routing = insn.data[1];
	return 0;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_counter_mode,comedi_set_counter_mode,0.9.0);
int _comedi_set_counter_mode(comedi_t *device, unsigned subdevice, unsigned channel, unsigned mode_bits)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_COUNTER_MODE;
	data[1] = mode_bits;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_clock_source,comedi_set_clock_source,0.10.0);
int _comedi_set_clock_source(comedi_t *device, unsigned subdevice, unsigned channel, unsigned clock, unsigned period_ns)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_CLOCK_SRC;
	data[1] = clock;
	data[2] = period_ns;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_VER(_comedi_set_clock_source_chan0,comedi_set_clock_source,0.9.0);
int _comedi_set_clock_source_chan0(comedi_t *device, unsigned subdevice, unsigned clock, unsigned period_ns)
{
	return _comedi_set_clock_source(device, subdevice, 0, clock, period_ns);
}

EXPORT_ALIAS_DEFAULT(_comedi_set_filter,comedi_set_filter,0.9.0);
int _comedi_set_filter(comedi_t *device, unsigned subdevice, unsigned channel, unsigned filter)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_FILTER;
	data[1] = filter;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_gate_source,comedi_set_gate_source,0.9.0);
int _comedi_set_gate_source(comedi_t *device, unsigned subdevice, unsigned channel,
	unsigned gate_index, unsigned gate_source)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_GATE_SRC;
	data[1] = gate_index;
	data[2] = gate_source;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_internal_trigger,comedi_internal_trigger,0.9.0);
int comedi_internal_trigger(comedi_t *dev, unsigned subd, unsigned trignum)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_INTTRIG;
	insn.subdev = subd;
	insn.data = data;
	insn.n = 1;

	data[0] = trignum;

	if(comedi_do_insn(dev, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_other_source,comedi_set_other_source,0.9.0);
int _comedi_set_other_source(comedi_t *device, unsigned subdevice,
	unsigned channel, unsigned other, unsigned source)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_OTHER_SRC;
	data[1] = other;
	data[2] = source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_routing,comedi_set_routing,0.9.0);
int _comedi_set_routing(comedi_t *device, unsigned subdevice, unsigned channel, unsigned routing)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_ROUTING;
	data[1] = routing;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_hardware_buffer_size,comedi_get_hardware_buffer_size,0.9.0);
int _comedi_get_hardware_buffer_size(comedi_t *device, unsigned subdevice, enum comedi_io_direction direction)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	memset(insn.data, 0, insn.n * sizeof(insn.data[0]));
	data[0] = INSN_CONFIG_GET_HARDWARE_BUFFER_SIZE;
	data[1] = direction;

	if(comedi_do_insn(device, &insn) >= 0) return data[2];
	else return -1;
}

static int digital_trigger_config(comedi_t *device, unsigned subdevice,
	unsigned trigger_id, unsigned config_operation,
	unsigned config_param1, unsigned config_param2, unsigned config_param3)
{
	comedi_insn insn;
	lsampl_t data[6];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_DIGITAL_TRIG;
	data[1] = trigger_id;
	data[2] = config_operation;
	data[3] = config_param1;
	data[4] = config_param2;
	data[5] = config_param3;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_digital_trigger_disable,comedi_digital_trigger_disable,0.11.0);
int _comedi_digital_trigger_disable(comedi_t *device, unsigned subdevice,
	unsigned trigger_id)
{
	return digital_trigger_config(device, subdevice, trigger_id,
		COMEDI_DIGITAL_TRIG_DISABLE, 0, 0, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_digital_trigger_enable_edges,comedi_digital_trigger_enable_edges,0.11.0);
int _comedi_digital_trigger_enable_edges(comedi_t *device, unsigned subdevice,
	unsigned trigger_id, unsigned base_input,
	unsigned rising_edge_inputs, unsigned falling_edge_inputs)
{
	return digital_trigger_config(device, subdevice, trigger_id,
		COMEDI_DIGITAL_TRIG_ENABLE_EDGES, base_input,
		rising_edge_inputs, falling_edge_inputs);
}

EXPORT_ALIAS_DEFAULT(_comedi_digital_trigger_enable_levels,comedi_digital_trigger_enable_levels,0.11.0);
int _comedi_digital_trigger_enable_levels(comedi_t *device, unsigned subdevice,
	unsigned trigger_id, unsigned base_input,
	unsigned high_level_inputs, unsigned low_level_inputs)
{
	return digital_trigger_config(device, subdevice, trigger_id,
		COMEDI_DIGITAL_TRIG_ENABLE_LEVELS, base_input,
		high_level_inputs, low_level_inputs);
}

static int _comedi_route_operation(comedi_t *device,
				   unsigned source, unsigned destination,
				   enum configuration_ids op,
				   int return_data)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_DEVICE_CONFIG;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = op;
	data[1] = source;
	data[2] = destination;

	if(comedi_do_insn(device, &insn) >= 0)
		return return_data ? data[0] : 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_test_route,comedi_test_route,0.12.0);
int _comedi_test_route(comedi_t *device, unsigned source, unsigned destination)
{
	return _comedi_route_operation(device, source, destination,
				       INSN_DEVICE_CONFIG_TEST_ROUTE, 1);
}

EXPORT_ALIAS_DEFAULT(_comedi_connect_route,comedi_connect_route,0.12.0);
int _comedi_connect_route(comedi_t *device, unsigned source, unsigned destination)
{
	return _comedi_route_operation(device, source, destination,
				       INSN_DEVICE_CONFIG_CONNECT_ROUTE, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_disconnect_route,comedi_disconnect_route,0.12.0);
int _comedi_disconnect_route(comedi_t *device, unsigned source, unsigned destination)
{
	return _comedi_route_operation(device, source, destination,
				       INSN_DEVICE_CONFIG_DISCONNECT_ROUTE, 0);
}

EXPORT_ALIAS_DEFAULT(_comedi_get_routes,comedi_get_routes,0.12.0);
int _comedi_get_routes(comedi_t *device, comedi_route_pair * routelist, size_t n)
{
	comedi_insn insn;
	int retval = -1;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_DEVICE_CONFIG;
	insn.n = 2 + 2*(routelist == NULL ? 0 : n);
	insn.data = (lsampl_t *)calloc(sizeof(lsampl_t), insn.n);
	insn.data[0] = INSN_DEVICE_CONFIG_GET_ROUTES;
	insn.data[1] = 0; /* number pairs copied to user. set on output */

	if(comedi_do_insn(device, &insn) >= 0) {
		retval = insn.data[1];
		for (unsigned int i = 0; i < n && i < retval; ++i) {
			routelist[i].source	 = insn.data[2 + 2*i];
			routelist[i].destination = insn.data[2 + 2*i + 1];
		}
	}

	free(insn.data);
	return retval;
}

EXPORT_ALIAS_DEFAULT(_comedi_get_cmd_timing_constraints,comedi_get_cmd_timing_constraints,0.12.0);
int _comedi_get_cmd_timing_constraints(comedi_t *device, unsigned int subdevice,
				       unsigned int scan_begin_src,
				       unsigned int *scan_begin_min,
				       unsigned int convert_src,
				       unsigned int *convert_min,
				       unsigned int *chanlist,
				       unsigned int chanlist_len)
{
	comedi_insn insn;
	size_t i;
	int retval = -1;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn   = INSN_CONFIG;
	insn.n	    = 4 + chanlist_len;
	insn.subdev = subdevice;
	insn.data   = calloc(sizeof(lsampl_t), insn.n);
	insn.data[0]= INSN_CONFIG_GET_CMD_TIMING_CONSTRAINTS;
	insn.data[1]= scan_begin_src; // scan_begin_min on return
	insn.data[2]= convert_src;    // convert_min on return
	insn.data[3]= chanlist_len;

	for (i = 0; i < chanlist_len; ++i) {
		insn.data[4+i] = chanlist[i];
	}

	if(comedi_do_insn(device, &insn) >= 0) {
		if (scan_begin_min)
			*scan_begin_min = insn.data[1];
		if (convert_min)
			*convert_min    = insn.data[2];
		retval = 0;
	}

	free(insn.data);
	return retval;
}
