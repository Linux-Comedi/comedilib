/*
   calibration support for NI m-series boards

   copyright (C) 2003, 2006 by Frank Mori Hess

 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "calib.h"

char ni_m_series_id[] = "$Id$";

int ni_m_series_setup(calibration_setup_t *setup , const char *device_name);
static int cal_ni_m_series(calibration_setup_t *setup);

struct board_struct{
	char *name;
	int status;
};

static struct board_struct boards[]={
	{ "pci-6289", STATUS_UNKNOWN},
};

static const int n_boards = sizeof(boards) / sizeof(boards[0]);

static struct board_struct* ni_board(calibration_setup_t *setup)
{
	return setup->private_data;
}

uint16_t ni_read_eeprom_u16(calibration_setup_t *setup, int offset)
{
	uint16_t value = read_eeprom(setup, offset) << 8;
	value |= read_eeprom(setup, offset + 1) & 0xff;
	return value;
}

float ni_read_eeprom_f32(calibration_setup_t *setup, int offset)
{
	union float_converter
	{
		uint32_t raw;
		float converted;
	};
	union float_converter my_converter;
	
	assert(sizeof(float) == 4);
	my_converter.raw = read_eeprom(setup, offset++) << 24 & 0xff000000;
	my_converter.raw |= (read_eeprom(setup, offset++) << 16) & 0xff0000;
	my_converter.raw |= (read_eeprom(setup, offset++) << 8) & 0xff00;
	my_converter.raw |= read_eeprom(setup, offset++) & 0xff;
	return my_converter.converted;
}

static void ni_m_series_setup_observables(calibration_setup_t *setup)
{
	static const int positive_cal_shift = 7;
	static const int negative_cal_shift = 10;
	enum positive_cal_source
	{
		POS_CAL_GROUND = 0 << positive_cal_shift,
		POS_CAL_REF = 2 << positive_cal_shift,
		POS_CAL_500mV = 3 << positive_cal_shift,
		POS_CAL_2V = 4 << positive_cal_shift,
		POS_CAL_10V = 5 << positive_cal_shift,
		POS_CAL_AO = 7 << positive_cal_shift
	};
	enum negative_cal_source
	{
		NEG_CAL_GROUND = 0 << negative_cal_shift,
		NEG_CAL_850mV = 2 << negative_cal_shift,
		NEG_CAL_10V = 7 << negative_cal_shift,
	};
	static const int calibration_area_offset = 24;
	int calibration_area_start;
	static const int voltage_reference_offset = 12;
	comedi_insn tmpl;
	double voltage_reference;
	observable *o = setup->observables;
	
	calibration_area_start = ni_read_eeprom_u16(setup, calibration_area_offset);
	DPRINT(1, "calibration area starts at offset %i\n", calibration_area_start);
	voltage_reference = ni_read_eeprom_f32(setup, calibration_area_start + voltage_reference_offset);
	DPRINT(1, "eeprom says voltage reference is %g V.\n", voltage_reference);

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	o = setup->observables;
	
	o->name = "ai, reference voltage source";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_GROUND | POS_CAL_REF;
	o->target = voltage_reference;
	++o;
	
	o->name = "ai, nominal 10V";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_GROUND | POS_CAL_10V;
	o->target = 10.;
	++o;

	o->name = "ai, nominal 2V";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_GROUND | POS_CAL_2V;
	o->target = 2.;
	++o;
		
	o->name = "ai, nominal 0.5V";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_GROUND | POS_CAL_500mV;
	o->target = 0.5;
	++o;

	o->name = "ai, ground";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_GROUND | POS_CAL_GROUND;
	o->target = 0.0;
	++o;

	o->name = "ai, nominal -0.85V";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_850mV | POS_CAL_GROUND;
	o->target = -0.85;
	++o;

	o->name = "ai, nominal -10V";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0, 0, AREF_DIFF) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = NEG_CAL_10V | POS_CAL_GROUND;
	o->target = -10.;
	++o;

	setup->n_observables = o - setup->observables;
}

int ni_m_series_setup(calibration_setup_t *setup , const char *device_name)
{	
	int i;
	for(i = 0; i < n_boards; ++i)
	{
		if(!strcmp(device_name, boards[i].name))
		{
			setup->status = boards[i].status;
			setup->private_data = &boards[i];
			setup->do_cal = &cal_ni_m_series;
			ni_m_series_setup_observables(setup);
			break;
		}
	}
	if(i == n_boards) return -1;

	return 0;
}

static int cal_ni_m_series(calibration_setup_t *setup)
{
	return 0;
}

