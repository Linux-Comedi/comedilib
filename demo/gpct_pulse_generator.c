/*
 * NI general-purpose counter example.  Configures the counter to
 * produce a continuous pulse train.  The argument specifies the
 * number of nanoseconds the output pulse should remain high.
 * The example assumes the board has already been configured to
 * route the output signal of the counter to an appropriate
 * location (you may need to route it to a PFI output line
 * for example).
 * Part of Comedilib
 *
 * Copyright (c) 2007 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a National Instruments general-purpose
 * counter, and comedi driver version 0.7.74 or newer.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"

/*FIXME: move helper functions to common.c so they can be used on other (mostly counter) examples*/

int arm(comedi_t *device, unsigned subdevice, lsampl_t source)
{
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_ARM;
	data[1] = source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

/* This resets the count to zero and disarms the counter.  The counter output
 is set low. */
int reset_counter(comedi_t *device, unsigned subdevice)
{
	comedi_insn insn;
	lsampl_t data[1];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_RESET;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits)
{
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_COUNTER_MODE;
	data[1] = mode_bits;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_CLOCK_SRC;
	data[1] = clock;
	data[2] = period_ns;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_GATE_SRC;
	data[1] = gate_index;
	data[2] = gate_source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int ni_gpct_start_pulse_generator(comedi_t *device, unsigned subdevice, unsigned period_ns, unsigned up_time_ns)
{
	int retval;
	lsampl_t counter_mode;
	const unsigned clock_period_ns = 50;	/* 20MHz clock */
	unsigned up_ticks, down_ticks;
/*
FIXME:
How is output initialized to known state for toggling (reset?)
*/
	retval = reset_counter(device, subdevice);
	if(retval < 0) return retval;

	retval = set_gate_source(device, subdevice, 0, NI_GPCT_DISABLED_GATE_SELECT | CR_EDGE);
	if(retval < 0) return retval;
	retval = set_gate_source(device, subdevice, 1, NI_GPCT_DISABLED_GATE_SELECT | CR_EDGE);
	if(retval < 0)
	{
		fprintf(stderr, "Failed to set second gate source.  This is expected for older boards (e-series, etc.)\n"
			"that don't have a second gate.\n");
	}

	counter_mode = NI_GPCT_COUNTING_MODE_NORMAL_BITS;
	// toggle output on terminal count
	counter_mode |= NI_GPCT_OUTPUT_TC_TOGGLE_BITS;
	// load on terminal count
	counter_mode |= NI_GPCT_LOADING_ON_TC_BIT;
	// alternate the reload source between the load a and load b registers
	counter_mode |= NI_GPCT_RELOAD_SOURCE_SWITCHING_BITS;
	// count down
	counter_mode |= NI_GPCT_COUNTING_DIRECTION_DOWN_BITS;
	// initialize load source as load b register
	counter_mode |= NI_GPCT_LOAD_B_SELECT_BIT;
	retval = set_counter_mode(device, subdevice, counter_mode);
	if(retval < 0) return retval;

	/* 20MHz clock */
	retval = set_clock_source(device, subdevice, NI_GPCT_TIMEBASE_1_CLOCK_SRC_BITS, clock_period_ns);
	if(retval < 0) return retval;

	up_ticks = (up_time_ns + clock_period_ns / 2) / clock_period_ns;
	down_ticks = (period_ns + clock_period_ns / 2) / clock_period_ns - up_ticks;
	/* set initial counter value by writing to channel 0 */
	retval = comedi_data_write(device, subdevice, 0, 0, 0, down_ticks);
	if(retval < 0) return retval;
	/* set "load a" register to the number of clock ticks the counter output should remain low
	by writing to channel 1. */
	comedi_data_write(device, subdevice, 1, 0, 0, down_ticks);
	if(retval < 0) return retval;
	/* set "load b" register to the number of clock ticks the counter output should remain high
	by writing to channel 2 */
	comedi_data_write(device, subdevice, 2, 0, 0, up_ticks);
	if(retval < 0) return retval;

	retval = arm(device, subdevice, NI_GPCT_ARM_IMMEDIATE);
	if(retval < 0) return retval;

	return 0;
}

int main(int argc, char *argv[])
{
	comedi_t *device;
	unsigned up_time;
	unsigned period_ns;
	int retval;
	struct parsed_options options;

	init_parsed_options(&options);
	options.value = -1.;
	parse_options(&options, argc, argv);
	period_ns = lrint(1e9 / options.freq);
	if(options.value < 0.)
		up_time = period_ns / 2;
	else
		up_time = lrint(options.value);
	device = comedi_open(options.filename);
	if(!device)
	{
		comedi_perror(options.filename);
		exit(-1);
	}
	/*FIXME: check that device is counter */
	printf("Generating pulse train on subdevice %d.\n", options.subdevice);
	printf("Period = %d ns.\n", period_ns);
	printf("Up Time = %d ns.\n", up_time);
	printf("Down Time = %d ns.\n", period_ns - up_time);

	retval = ni_gpct_start_pulse_generator(device, options.subdevice, period_ns, up_time);
	if(retval < 0) return retval;
	return 0;
}

