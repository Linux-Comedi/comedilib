/***************************************************************************
	cb.c  -  calibration support for some Measurement computing boards.
	Based on ni.c by David Schleef.
                             -------------------

    begin                : Sat Apr 27 2002
    copyright            : (C) 2002,2003 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net

 ***************************************************************************/

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
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "calib.h"


char cb_id[] = "$Id$";

struct board_struct{
	char *name;
	int status;
	int (*setup)( calibration_setup_t *setup );
};

static int setup_cb_pci_1xxx( calibration_setup_t *setup );
static int setup_cb_pci_1001( calibration_setup_t *setup );
static int setup_cb_pci_1602_16( calibration_setup_t *setup );

static int cal_cb_pci_1xxx( calibration_setup_t *setup );
static int cal_cb_pci_1001( calibration_setup_t *setup );
static int cal_cb_pci_1602_16( calibration_setup_t *setup );

static int init_observables_1xxx( calibration_setup_t *setup );
static int init_observables_1001( calibration_setup_t *setup );
static int init_observables_1602_16( calibration_setup_t *setup );

static struct board_struct boards[]={
	{ "pci-das1000",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1001",	STATUS_GUESS,	setup_cb_pci_1001 },
	{ "pci-das1002",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1200",	STATUS_DONE,	setup_cb_pci_1xxx },
	{ "pci-das1200/jr",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/12",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/16",	STATUS_GUESS,	setup_cb_pci_1602_16 },
	{ "pci-das1602/16/jr",	STATUS_GUESS,	setup_cb_pci_1602_16 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum observables_1xxx {
	OBS_0V_RANGE_10V_BIP_1XXX = 0,
	OBS_7V_RANGE_10V_BIP_1XXX,
	OBS_DAC0_GROUND_1XXX,
	OBS_DAC0_HIGH_1XXX,
	OBS_DAC1_GROUND_1XXX,
	OBS_DAC1_HIGH_1XXX,
};

enum observables_1602_16 {
	OBS_0V_RANGE_10V_BIP_1602_16 = 0,
	OBS_7V_RANGE_10V_BIP_1602_16,
};

enum calibration_source_1xxx
{
	CS_1XXX_GROUND = 0,
	CS_1XXX_7V = 1,
	CS_1XXX_3500mV = 2,
	CS_1XXX_1750mV = 3,
	CS_1XXX_875mV = 4,
	CS_1XXX_8600uV = 5,
	CS_1XXX_DAC0 = 6,
	CS_1XXX_DAC1 = 7,
};
static inline int CS_1XXX_DAC( unsigned int channel )
{
	if( channel )
		return CS_1XXX_DAC1;
	else
		return CS_1XXX_DAC0;
}

enum cal_knobs_1xxx
{
	DAC0_GAIN_FINE_1XXX = 0,
	DAC0_GAIN_COARSE_1XXX = 1,
	DAC0_OFFSET_1XXX = 2,
	DAC1_OFFSET_1XXX = 3,
	DAC1_GAIN_FINE_1XXX = 4,
	DAC1_GAIN_COARSE_1XXX = 5,
	ADC_OFFSET_COARSE_1XXX = 6,
	ADC_OFFSET_FINE_1XXX = 7,
	ADC_GAIN_1XXX = 8,
};
static inline unsigned int DAC_OFFSET_1XXX( unsigned int channel )
{
	if( channel )
		return DAC1_OFFSET_1XXX;
	else
		return DAC0_OFFSET_1XXX;
}
static inline unsigned int DAC_GAIN_FINE_1XXX( unsigned int channel )
{
	if( channel )
		return DAC1_GAIN_FINE_1XXX;
	else
		return DAC0_GAIN_FINE_1XXX;
}
static inline unsigned int DAC_GAIN_COARSE_1XXX( unsigned int channel )
{
	if( channel )
		return DAC1_GAIN_COARSE_1XXX;
	else
		return DAC0_GAIN_COARSE_1XXX;
}

int cb_setup( calibration_setup_t *setup, const char *device_name )
{
	unsigned int i;

	for( i = 0; i < num_boards; i++ )
	{
		if( !strcmp( devicename, boards[i].name ) )
		{
			setup->status = boards[i].status;
			return boards[i].setup( setup );
			break;
		}
	}
	if( i == num_boards ) return -1;

	return 0;
}

static int setup_cb_pci_1xxx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;

	init_observables_1xxx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_1xxx;
	return 0;
}

static int setup_cb_pci_1001( calibration_setup_t *setup )
{
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;

	init_observables_1001( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_1001;
	return 0;
}

static int setup_cb_pci_1602_16( calibration_setup_t *setup )
{
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;
	static const int dac08_subdev = 6;

	init_observables_1602_16( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup_caldacs( setup, dac08_subdev );
	setup->do_cal = cal_cb_pci_1602_16;
	return 0;
}

static unsigned int ai_low_observable_1xxx( unsigned int range )
{
	return 2 * range;
}

static unsigned int ai_high_observable_1xxx( unsigned int range )
{
	return ai_low_observable_1xxx( range ) + 1;
}

static unsigned int ao_low_observable_1xxx( calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	int num_ai_ranges, num_ao_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges > 0 );
	num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
	assert( num_ao_ranges > 0 );

	return 2 * num_ai_ranges + 2 * num_ao_ranges * channel + 2 * range;
}

static unsigned int ao_high_observable_1xxx( calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	return ao_low_observable_1xxx( setup, channel, range ) + 1;
}

static double ai_low_target_1xxx( calibration_setup_t *setup,
	unsigned int range_index )
{
	comedi_range *range;
	int max_data;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	assert( range != NULL );
	max_data = comedi_get_maxdata( setup->dev, setup->ad_subdev, 0 );
	assert( max_data != 0 );

	if( range->min < -0.0001 )
		return 0.0;
	else	// return half a bit above zero for unipolar ranges
		return comedi_to_phys( 1, range, max_data ) / 2.0;
}

static int source_eeprom_addr_1xxx( calibration_setup_t *setup, unsigned int range_index )
{
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x80,
		EEPROM_3500mV_CHAN = 0x84,
		EEPROM_1750mV_CHAN = 0x88,
		EEPROM_875mV_CHAN = 0x8c,
		EEPROM_8600uV_CHAN = 0x90,
	};
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return EEPROM_7V_CHAN;
	else if( range->max > 3.5 )
		return EEPROM_3500mV_CHAN;
	else if( range->max > 1.750 )
		return EEPROM_1750mV_CHAN;
	else if( range->max > 0.875 )
		return EEPROM_875mV_CHAN;
	else if( range->max > 0.0086 )
		return EEPROM_8600uV_CHAN;

	return -1;
}

static int ai_high_cal_source_1xxx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return CS_1XXX_7V;
	else if( range->max > 3.5 )
		return CS_1XXX_3500mV;
	else if( range->max > 1.750 )
		return CS_1XXX_1750mV;
	else if( range->max > 0.875 )
		return CS_1XXX_875mV;
	else if( range->max > 0.0086 )
		return CS_1XXX_8600uV;

	return -1;
}

static int ao_set_high_target_1xxx( calibration_setup_t *setup, unsigned int obs,
	unsigned int range_index )
{
	double target;
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->da_subdev, 0, range_index );
	if( range == NULL ) return -1;

	target = range->max * 0.9;
	set_target( setup, obs, target );
	return 0;
}

static int init_observables_1xxx( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	int retval, range, num_ai_ranges, num_ao_ranges,
		channel, num_channels;
	float target;
	int ai_for_ao_range;

	setup->n_observables = 0;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		o = setup->observables + ai_low_observable_1xxx( range );
		o->reference_source = CS_1XXX_GROUND;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = ai_low_target_1xxx( setup, range );
		setup->n_observables++;

		o = setup->observables + ai_high_observable_1xxx( range );;
		retval = ai_high_cal_source_1xxx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
			source_eeprom_addr_1xxx( setup, range ), &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}

	if( setup->da_subdev >= 0 )
	{
		num_channels = comedi_get_n_channels( setup->dev, setup->da_subdev );
		if( num_channels < 0 ) return -1;

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		ai_for_ao_range = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
		if( ai_for_ao_range < 0 ) return -1;

		for( range = 0; range < num_ao_ranges; range++ )
		{
			for( channel = 0; channel < num_channels; channel++ )
			{
				o = setup->observables + ao_low_observable_1xxx( setup, channel, range );
				o->reference_source = CS_1XXX_DAC( channel );
				assert( o->name == NULL );
				asprintf( &o->name, "DAC ground calibration source, ch %i, range %i",
					channel, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( channel, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_for_ao_range, AREF_GROUND) |
					CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_low_observable_1xxx( setup, channel, range ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_1xxx( setup, channel, range );
				o->reference_source = CS_1XXX_DAC( channel );
				assert( o->name == NULL );
				asprintf( &o->name, "DAC high calibration source, ch %i, range %i", channel,
					range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( channel , range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_for_ao_range, AREF_GROUND) |
					CR_ALT_SOURCE | CR_ALT_FILTER;
				ao_set_high_target_1xxx( setup, ao_high_observable_1xxx( setup, channel, range ),
					range );
				setup->n_observables++;
			}
		}
	}

	return 0;
}

static int init_observables_1001( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	int retval;
	float target;
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x80,
		EEPROM_88600uV_CHAN = 0x88,
		EEPROM_875mV_CHAN = 0x8c,
		EEPROM_8600uV_CHAN = 0x90,
	};
	enum calibration_source
	{
		CAL_SRC_GROUND = 0,
		CAL_SRC_7V = 1,
		CAL_SRC_88600uV = 3,
		CAL_SRC_875mV = 4,
		CAL_SRC_8600uV = 5,
		CAL_SRC_DAC0 = 6,
		CAL_SRC_DAC1 = 7,
	};
	enum ai_ranges
	{
		AI_RNG_BIP_10V = 0
	};
	enum ao_ranges
	{
		AO_RNG_BIP_10V = 1,
	};

	setup->n_observables = 0;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	o = setup->observables + OBS_0V_RANGE_10V_BIP_1XXX;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
		CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;
	setup->n_observables++;

	o = setup->observables + OBS_7V_RANGE_10V_BIP_1XXX;
	o->name = "7V calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_7V;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
		CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 7.0;
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
	if( retval == 0 )
		o->target = target;
	setup->n_observables++;

	if( setup->da_subdev >= 0 )
	{
		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		o = setup->observables + OBS_DAC0_GROUND_1XXX;
		o->name = "DAC0 ground calibration source, 10V bipolar input range";
		o->reference_source = CAL_SRC_DAC0;
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( 0, AO_RNG_BIP_10V, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		set_target( setup, OBS_DAC0_GROUND_1XXX, 0.0 );
		setup->n_observables++;

		o = setup->observables + OBS_DAC0_HIGH_1XXX;
		o->name = "DAC0 high calibration source, 10V bipolar input range";
		o->reference_source = CAL_SRC_DAC0;
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( 0 , AO_RNG_BIP_10V, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		set_target( setup, OBS_DAC0_HIGH_1XXX, 8.0 );
		setup->n_observables++;

		o = setup->observables + OBS_DAC1_GROUND_1XXX;
		o->name = "DAC1 ground calibration source, 10V bipolar input range";
		o->reference_source = CAL_SRC_DAC1;
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( 1, AO_RNG_BIP_10V, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		set_target( setup, OBS_DAC1_GROUND_1XXX, 0.0 );
		setup->n_observables++;

		o = setup->observables + OBS_DAC1_HIGH_1XXX;
		o->name = "DAC1 high calibration source, 10V bipolar input range";
		o->reference_source = CAL_SRC_DAC1;
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK( 1 , AO_RNG_BIP_10V, AREF_GROUND );
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, AI_RNG_BIP_10V, AREF_GROUND) |
			CR_ALT_SOURCE | CR_ALT_FILTER;
		set_target( setup, OBS_DAC1_HIGH_1XXX, 8.0 );
		setup->n_observables++;
	}

	return 0;
}

static int init_observables_1602_16( calibration_setup_t *setup )
{
	comedi_insn tmpl;//, po_tmpl;
	observable *o;
#if 0
// XXX need to figure out eeprom map
	int retval;
	float target;
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x30,
		EEPROM_3500mV_CHAN = 0x32,
		EEPROM_1750mV_CHAN = 0x34,
		EEPROM_875mV_CHAN = 0x36,
		EEPROM_8600uV_CHAN = 0x38,
	};
#endif
	enum calibration_source
	{
		CAL_SRC_GROUND = 0,
		CAL_SRC_7V = 1,
		CAL_SRC_3500mV = 2,
		CAL_SRC_1750mV = 3,
		CAL_SRC_875mV = 4,
		CAL_SRC_minus_10V = 5,
		CAL_SRC_DAC0 = 6,
		CAL_SRC_DAC1 = 7,
	};
#if 0
	memset( &po_tmpl, 0, sizeof(po_tmpl) );
	po_tmpl.insn = INSN_CONFIG;
	po_tmpl.n = 2;
	po_tmpl.subdev = setup->ad_subdev;
#endif
	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	o = setup->observables + OBS_0V_RANGE_10V_BIP_1602_16;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + OBS_7V_RANGE_10V_BIP_1602_16;
	o->name = "7V calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_7V;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 7.0;
#if 0
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
	if( retval == 0 )
		o->target = target;
#endif
	setup->n_observables = 2;

	return 0;
}

static void prep_adc_caldacs_1xxx( calibration_setup_t *setup,
	unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		reset_caldac( setup, ADC_OFFSET_COARSE_1XXX );
		reset_caldac( setup, ADC_OFFSET_FINE_1XXX );
		reset_caldac( setup, ADC_GAIN_1XXX );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->ad_subdev,
			0, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			reset_caldac( setup, ADC_OFFSET_COARSE_1XXX );
			reset_caldac( setup, ADC_OFFSET_FINE_1XXX );
			reset_caldac( setup, ADC_GAIN_1XXX );
		}
	}
}

static void prep_dac_caldacs_1xxx( calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		reset_caldac( setup, DAC_OFFSET_1XXX( channel ) );
		reset_caldac( setup, DAC_GAIN_FINE_1XXX( channel ) );
		reset_caldac( setup, DAC_GAIN_COARSE_1XXX( channel ) );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			reset_caldac( setup, DAC_OFFSET_1XXX( channel ) );
			reset_caldac( setup, DAC_GAIN_FINE_1XXX( channel ) );
			reset_caldac( setup, DAC_GAIN_COARSE_1XXX( channel ) );
		}
	}
}

static int prep_ai_for_ao_1xxx( calibration_setup_t *setup,
	saved_calibration_t *saved_cals, unsigned int ao_range )
{
	int retval, ai_range, i, num_caldacs;
	caldac_t *dacs;

	retval = ao_low_observable_1xxx( setup, 0, ao_range );
	if( retval < 0 ) return -1;
	ai_range = CR_RANGE( setup->observables[ retval ].observe_insn.chanspec );

	dacs = saved_cals[ ai_range ].caldacs;
	num_caldacs = saved_cals[ ai_range ].caldacs_length;
	for( i = 0; i < num_caldacs; i++ )
	{
		retval = comedi_data_write( setup->dev, dacs[ i ].subdev, dacs[ i ].chan,
			0, 0, dacs[ i ].current );
		assert( retval >= 0 );
	}
	return 0;
}

static int cal_cb_pci_1xxx( calibration_setup_t *setup )
{
	saved_calibration_t *saved_cals, *current_cal;
	int range, num_ai_ranges, num_ao_ranges, num_calibrations,
		retval, channel, i;

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 1 ) return -1;

	if( setup->da_subdev >= 0 )
	{
		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ai_ranges < 1 ) return -1;
	}else
		num_ao_ranges = 0;

	num_calibrations = num_ai_ranges + 2 * num_ao_ranges;

	saved_cals = malloc( num_calibrations * sizeof( saved_calibration_t ) );
	if( saved_cals == NULL ) return -1;
	memset( saved_cals, 0, num_calibrations * sizeof( saved_calibration_t ) );

	current_cal = saved_cals;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		prep_adc_caldacs_1xxx( setup, range );

		cal_binary( setup, ai_low_observable_1xxx( range ), ADC_OFFSET_COARSE_1XXX );
		cal_binary( setup, ai_low_observable_1xxx( range ), ADC_OFFSET_FINE_1XXX );
		cal_binary( setup, ai_high_observable_1xxx( range ), ADC_GAIN_1XXX );

		current_cal->subdevice = setup->ad_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_COARSE_1XXX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_FINE_1XXX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_1XXX ] );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		sc_push_range( current_cal, range );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;
	}

	if( setup->da_subdev >= 0 )
	{
		for( range = 0; range < num_ao_ranges; range++ )
		{
			for( channel = 0; channel < 2; channel++ )
			{
				prep_ai_for_ao_1xxx( setup, saved_cals, range );

				prep_dac_caldacs_1xxx( setup, channel, range );

				cal_binary( setup, ao_low_observable_1xxx( setup, channel, range ),
					DAC_OFFSET_1XXX( channel ) );
				cal_binary( setup, ao_high_observable_1xxx( setup, channel, range ),
					DAC_GAIN_COARSE_1XXX( channel ) );
				cal_binary( setup, ao_high_observable_1xxx( setup, channel, range ),
					DAC_GAIN_FINE_1XXX( channel ) );

				current_cal->subdevice = setup->da_subdev;
				sc_push_caldac( current_cal, setup->caldacs[ DAC_OFFSET_1XXX( channel ) ] );
				sc_push_caldac( current_cal, setup->caldacs[ DAC_GAIN_COARSE_1XXX( channel ) ] );
				sc_push_caldac( current_cal, setup->caldacs[ DAC_GAIN_FINE_1XXX( channel ) ] );
				sc_push_channel( current_cal, channel );
				sc_push_range( current_cal, range );
				sc_push_aref( current_cal, SC_ALL_AREFS );

				current_cal++;
			}
		}
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

static int cal_cb_pci_1001( calibration_setup_t *setup )
{
	enum cal_knobs_1xxx
	{
		DAC0_GAIN_FINE = 0,
		DAC0_GAIN_COARSE,
		DAC0_OFFSET,
		DAC1_OFFSET,
		DAC1_GAIN_FINE,
		DAC1_GAIN_COARSE,
		ADC_OFFSET_COARSE,
		ADC_OFFSET_FINE,
		ADC_GAIN,
	};

	cal1( setup, OBS_0V_RANGE_10V_BIP_1XXX, ADC_OFFSET_COARSE );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_1XXX, ADC_OFFSET_COARSE );

	cal1( setup, OBS_0V_RANGE_10V_BIP_1XXX, ADC_OFFSET_FINE );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_1XXX, ADC_OFFSET_FINE );

	cal1( setup, OBS_7V_RANGE_10V_BIP_1XXX, ADC_GAIN );
	cal1_fine( setup, OBS_7V_RANGE_10V_BIP_1XXX, ADC_GAIN );

	if( setup->da_subdev >= 0 )
	{
		cal1( setup, OBS_DAC0_GROUND_1XXX, DAC0_OFFSET );
		cal1( setup, OBS_DAC0_HIGH_1XXX, DAC0_GAIN_COARSE );
		cal1( setup, OBS_DAC0_HIGH_1XXX, DAC0_GAIN_FINE );

		cal1( setup, OBS_DAC1_GROUND_1XXX, DAC1_OFFSET );
		cal1( setup, OBS_DAC1_HIGH_1XXX, DAC1_GAIN_COARSE );
		cal1( setup, OBS_DAC1_HIGH_1XXX, DAC1_GAIN_FINE );
	}

	return 0;
}

static int cal_cb_pci_1602_16( calibration_setup_t *setup )
{
	enum cal_knobs_1602_16
	{
		DAC0_GAIN_FINE = 0,
		DAC0_GAIN_COARSE,
		DAC0_OFFSET_COARSE,
		DAC1_OFFSET_COARSE,
		DAC1_GAIN_FINE,
		DAC1_GAIN_COARSE,
		DAC0_OFFSET_FINE,
		DAC1_OFFSET_FINE,
		ADC_GAIN,
		ADC_POSTGAIN_OFFSET,
		ADC_PREGAIN_OFFSET,
	};

	cal1( setup, OBS_0V_RANGE_10V_BIP_1602_16, ADC_PREGAIN_OFFSET );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_1602_16, ADC_PREGAIN_OFFSET );

	cal1( setup, OBS_0V_RANGE_10V_BIP_1602_16, ADC_POSTGAIN_OFFSET );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_1602_16, ADC_POSTGAIN_OFFSET );

	cal1( setup, OBS_7V_RANGE_10V_BIP_1602_16, ADC_GAIN );
	cal1_fine( setup, OBS_7V_RANGE_10V_BIP_1602_16, ADC_GAIN );

	return 0;
}

// converts calibration source voltages from two 16 bit eeprom values to a floating point value
static float eeprom16_to_source( uint16_t *data )
{
	union translator
	{
		uint32_t bits;
		float	value;
	};

	union translator my_translator;

	my_translator.bits = ( data[ 0 ] & 0xffff ) | ( ( data[ 1 ] << 16 ) & 0xffff0000 );

	return my_translator.value;
}

static float eeprom8_to_source( uint8_t *data )
{
	union translator
	{
		uint32_t bits;
		float	value;
	};
	union translator my_translator;
	int i;

	my_translator.bits = 0;
	for( i = 0; i < 4; i++ )
	{
		my_translator.bits |= ( data[ i ] & 0xffff ) << ( 8 * i );
	}

	return my_translator.value;
}

int cb_actual_source_voltage( comedi_t *dev, unsigned int subdevice, unsigned int eeprom_channel, float *voltage)
{
	int retval;
	unsigned int i;
	lsampl_t data;
	int max_data;

	max_data = comedi_get_maxdata( dev, subdevice, eeprom_channel );

	if( max_data == 0xffff )
	{
		uint16_t word[ 2 ];

		for( i = 0; i < 2; i++ )
		{
			retval = comedi_data_read( dev, subdevice, eeprom_channel + i, 0, 0, &data );
			if( retval < 0 )
			{
				comedi_perror( __FUNCTION__ );
				return retval;
			}
			word[ i ] = data;
		}
		*voltage = eeprom16_to_source( word );
	}else if( max_data == 0xff )
	{
		uint8_t byte[ 4 ];

		for( i = 0; i < 4; i++ )
		{
			retval = comedi_data_read( dev, subdevice, eeprom_channel + i, 0, 0, &data );
			if( retval < 0 )
			{
				comedi_perror( __FUNCTION__ );
				return retval;
			}
			byte[ i ] = data;
		}
		*voltage = eeprom8_to_source( byte );
	}else
	{
		fprintf( stderr, "%s: maxdata = 0x%x invalid\n",
			__FUNCTION__, max_data );
		return -1;
	}

	DPRINT(0, "eeprom ch 0x%x gives calibration source of %gV\n", eeprom_channel, *voltage);
	return 0;
}
