/***************************************************************************
	cb64.c  -  calibration support for some Measurement computing boards.
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


char cb64_id[] = "$Id$";

struct board_struct{
	char *name;
	int status;
	int (*setup)( calibration_setup_t *setup );
};

static int setup_cb_pci_64xx( calibration_setup_t *setup );
static int setup_cb_pci_60xx( calibration_setup_t *setup );
static int setup_cb_pci_4020( calibration_setup_t *setup );

static int cal_cb_pci_64xx( calibration_setup_t *setup );
static int cal_cb_pci_60xx( calibration_setup_t *setup );
static int cal_cb_pci_4020( calibration_setup_t *setup );

static int init_observables_64xx( calibration_setup_t *setup );
static int init_observables_60xx( calibration_setup_t *setup );
static int init_observables_4020( calibration_setup_t *setup );

static struct board_struct boards[]={
	{ "pci-das6402/16",	STATUS_SOME,	setup_cb_pci_64xx },
	{ "pci-das6402/12",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m1/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m2/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m3/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das6023",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6025",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6034",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6035",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das4020/12",	STATUS_SOME,	setup_cb_pci_4020 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum calibration_source_60xx
{
	CS_60XX_GROUND = 0,
	CS_60XX_10V = 1,
	CS_60XX_5V = 2,
	CS_60XX_500mV = 3,
	CS_60XX_50mV = 4,
	CS_60XX_UNUSED = 5,	// 0V
	CS_60XX_DAC0 = 6,
	CS_60XX_DAC1 = 7,
};

enum calibration_source_64xx
{
	CS_64XX_GROUND = 0,
	CS_64XX_7V = 1,
	CS_64XX_3500mV = 2,
	CS_64XX_1750mV = 3,
	CS_64XX_875mV = 4,
	CS_64XX_8600uV = 5,
	CS_64XX_DAC0 = 6,
	CS_64XX_DAC1 = 7,
};

enum cal_knobs_60xx
{
	DAC0_OFFSET_60XX = 0,
	DAC0_GAIN_60XX,
	DAC1_OFFSET_60XX,
	DAC1_GAIN_60XX,
	ADC_OFFSET_FINE_60XX,
	ADC_OFFSET_COARSE_60XX,
	ADC_GAIN_COARSE_60XX,
	ADC_GAIN_FINE_60XX,
};

enum cal_knobs_64xx
{
	DAC0_GAIN_FINE_64XX = 0,
	DAC0_GAIN_COARSE_64XX,
	DAC0_OFFSET_COARSE_64XX,
	DAC1_OFFSET_COARSE_64XX,
	DAC1_GAIN_FINE_64XX,
	DAC1_GAIN_COARSE_64XX,
	DAC0_OFFSET_FINE_64XX,
	DAC1_OFFSET_FINE_64XX,
	ADC_GAIN_64XX,
	ADC_OFFSET_64XX,
};

int cb64_setup( calibration_setup_t *setup, const char *device_name )
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

static int setup_cb_pci_64xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;
	static const int calpot_subdev = 7;

	init_observables_64xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_64xx;
	return 0;
}

static int setup_cb_pci_60xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;

	init_observables_60xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

static int setup_cb_pci_4020( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;

	init_observables_4020( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_4020;
	return 0;
}

static int ai_ground_observable_index_64xx( unsigned int range )
{
	return 2 * range;
}

static int ai_high_observable_index_64xx( unsigned int range )
{
	return 2 * range + 1;
}

static unsigned int ao_low_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range;
}

static unsigned int ao_high_observable_index_64xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range + 1;
}

static int source_eeprom_addr_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x30,
		EEPROM_3500mV_CHAN = 0x32,
		EEPROM_1750mV_CHAN = 0x34,
		EEPROM_875mV_CHAN = 0x36,
		EEPROM_8600uV_CHAN = 0x38,
	};
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return EEPROM_7V_CHAN;
	else if( range->max > 3.5 )
		return EEPROM_3500mV_CHAN;
	else if( range->max > 0.875 )
		return EEPROM_875mV_CHAN;
	else if( range->max > 0.0086 )
		return EEPROM_8600uV_CHAN;

	return -1;
}

static int ai_low_cal_source_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->min > -0.001 )
		return CS_64XX_8600uV;
	else
		return CS_64XX_GROUND;
}

static int ai_high_cal_source_64xx( calibration_setup_t *setup, unsigned int range_index )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, range_index );
	if( range == NULL ) return -1;

	if( range->max > 7.0 )
		return CS_64XX_7V;
	else if( range->max > 3.5 )
		return CS_64XX_3500mV;
	else if( range->max > 0.875 )
		return CS_64XX_875mV;
	else if( range->max > 0.0086 )
		return CS_64XX_8600uV;

	return -1;
}

static int ao_cal_src_64xx( unsigned int channel )
{
	switch( channel )
	{
	case 0:
		return CS_64XX_DAC0;
		break;
	case 1:
		return CS_64XX_DAC1;
		break;
	default:
		return -1;
		break;
	}
}

static int ao_set_high_target_64xx( calibration_setup_t *setup, unsigned int obs,
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

static int init_observables_64xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	int retval;
	float target;
	int range, num_ai_ranges, num_ao_ranges;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 0 ) return -1;

	setup->n_observables = 0;

	for( range = 0; range < num_ai_ranges; range++ )
	{
		o = setup->observables + ai_ground_observable_index_64xx( range );
		retval = ai_low_cal_source_64xx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = 0.0;
		setup->n_observables++;

		o = setup->observables + ai_high_observable_index_64xx( range );
		retval = ai_high_cal_source_64xx( setup, range );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, range );
		o->name = "calibration source, 10V bipolar range, ground referenced";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
			source_eeprom_addr_64xx( setup, range ), &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}
	if( setup->da_subdev >= 0 )
	{
		comedi_insn po_tmpl;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ao_ranges < 0 ) return -1;

		for( range = 0; range < num_ao_ranges; range++ )
		{
			int dac_chan, ai_range;
			ai_range = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
			if( ai_range < 0 ) return -1;

			for( dac_chan = 0; dac_chan < 2; dac_chan++ )
			{
				o = setup->observables + ao_low_observable_index_64xx( setup,
					dac_chan, range );
				o->reference_source = ao_cal_src_64xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i low, range %i, ground referenced", dac_chan, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_low_observable_index_64xx( setup, dac_chan, range ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_index_64xx( setup, dac_chan, range );
				o->reference_source = ao_cal_src_64xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i high, range %i, ground referenced", dac_chan, range );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, range, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, ai_range, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				retval = ao_set_high_target_64xx( setup,
					ao_high_observable_index_64xx( setup, dac_chan, range ), range );
				if( retval < 0 ) return -1;
				setup->n_observables++;
			}
		}
	}

	return 0;
}

static unsigned int ai_high_observable_index_60xx( unsigned int ai_range )
{
	return ai_range * 2 + 1;
}

static unsigned int ai_ground_observable_index_60xx( unsigned int ai_range )
{
	return ai_range * 2;
}

static unsigned int ao_high_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range + 1;
}

static unsigned int ao_low_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range;
}

static int ai_cal_src_voltage_60xx( calibration_setup_t *setup,
	unsigned int calibration_source, float *voltage )
{
	enum source_eeprom_addr
	{
		EEPROM_10V_CHAN = 0x30,
		EEPROM_5V_CHAN = 0x32,
		EEPROM_500mV_CHAN = 0x38,
		EEPROM_50mV_CHAN = 0x3e,
		EEPROM_8mV_CHAN = 0x40,	// bogus?
	};
	int retval;

	switch( calibration_source )
	{
		case CS_60XX_GROUND:
			*voltage = 0.0;
			retval = 0;
			break;
		case CS_60XX_10V:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_10V_CHAN, voltage );
			break;
		case CS_60XX_5V:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_5V_CHAN, voltage );
			break;
		case CS_60XX_500mV:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_500mV_CHAN, voltage );
			break;
		case CS_60XX_50mV:
			retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_50mV_CHAN, voltage );
			break;
		default:
			fprintf( stderr, "invalid calibration_source in ai_cal_src_voltage_60xx()\n" );
			retval = -1;
			break;
	}
	return retval;
}

static int high_ai_cal_src_60xx( calibration_setup_t *setup, unsigned int ai_range )
{
	comedi_range *range;

	range = comedi_get_range( setup->dev, setup->ad_subdev, 0, ai_range );
	if( range == NULL ) return -1;

	if( range->max > 9.999 )
		return CS_60XX_10V;
	else if( range->max > 4.999 )
		return CS_60XX_5V;
	else if( range->max > 0.4999 )
		return CS_60XX_500mV;
	else if( range->max > 0.04999 )
		return CS_60XX_50mV;

	return -1;
}

static int ao_cal_src_60xx( unsigned int channel )
{
	switch( channel )
	{
	case 0:
		return CS_60XX_DAC0;
		break;
	case 1:
		return CS_60XX_DAC1;
		break;
	default:
		return -1;
		break;
	}
}

static int init_observables_60xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	observable *o;
	int retval, num_ranges, i;
	float target;
	enum
	{
		CAL_SRC_GROUND = 0,
	};

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	setup->n_observables = 0;

	num_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ranges < 0 ) return -1;

	for( i = 0; i < num_ranges; i++ )
	{
		o = setup->observables + ai_ground_observable_index_60xx( i );
		o->reference_source = CAL_SRC_GROUND;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, i );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, i, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		o->target = 0.0;
		setup->n_observables++;

		o = setup->observables + ai_high_observable_index_60xx( i );
		retval = high_ai_cal_src_60xx( setup, i );
		if( retval < 0 ) return -1;
		o->reference_source = retval;
		assert( o->name == NULL );
		asprintf( &o->name, "calibration source %i, range %i, ground referenced",
			o->reference_source, i );
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK( 0, i, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
		retval = ai_cal_src_voltage_60xx( setup, o->reference_source, &target );
		if( retval < 0 ) return -1;
		o->target = target;
		setup->n_observables++;
	}

	if( setup->da_subdev >= 0 )
	{
		comedi_insn po_tmpl;

		memset( &po_tmpl, 0, sizeof(po_tmpl) );
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		num_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ranges < 0 ) return -1;

		for( i = 0; i < num_ranges; i++ )
		{
			int dac_chan;
			for( dac_chan = 0; dac_chan < 2; dac_chan++ )
			{
				o = setup->observables + ao_low_observable_index_60xx( setup,
					dac_chan, i );
				o->reference_source = ao_cal_src_60xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i low, range %i, ground referenced", dac_chan, i );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, i, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_low_observable_index_60xx( setup, dac_chan, i ), 0.0 );
				setup->n_observables++;

				o = setup->observables + ao_high_observable_index_60xx( setup, dac_chan, i );
				o->reference_source = ao_cal_src_60xx( dac_chan );
				assert( o->name == NULL );
				asprintf( &o->name, "dac%i high, range %i, ground referenced", dac_chan, i );
				o->preobserve_insn = po_tmpl;
				o->preobserve_insn.chanspec = CR_PACK( dac_chan, i, AREF_GROUND );
				o->preobserve_insn.data = o->preobserve_data;
				o->observe_insn = tmpl;
				o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
				set_target( setup, ao_high_observable_index_60xx( setup, dac_chan, i ), 9.0 );
				setup->n_observables++;
			}
		}
	}

	return 0;
}

static int init_observables_4020( calibration_setup_t *setup )
{
	comedi_insn tmpl;//, po_tmpl;
	observable *o;
	float target;
	int retval;
	enum source_eeprom_addr
	{
		EEPROM_4375mV_CHAN = 0x30,
		EEPROM_625mV_CHAN = 0x32,
	};
	enum calibration_source
	{
		CAL_SRC_4375mV = 5,
		CAL_SRC_625mV = 6,
		CAL_SRC_GROUND = 7,
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

	o = setup->observables + 0;
	o->name = "ground calibration source, ch 0, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;


	o = setup->observables + 1;
	o->name = "ground calibration source, ch 1, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + 2;
	o->name = "ground calibration source, ch 2, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 2, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + 3;
	o->name = "ground calibration source, ch 3, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 3, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + 4;
	o->name = "4.375V calibration source, ch 0, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 5;
	o->name = "4.375V calibration source, ch 1, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 6;
	o->name = "4.375V calibration source, ch 2, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 2, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 7;
	o->name = "4.375V calibration source, ch 3, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 3, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = cb_actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	setup->n_observables = 8;

	return 0;
}

static void prep_adc_caldacs_64xx( calibration_setup_t *setup, unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		reset_caldac( setup, ADC_OFFSET_64XX );
		reset_caldac( setup, ADC_GAIN_64XX );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->ad_subdev,
			0, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			reset_caldac( setup, ADC_OFFSET_64XX );
			reset_caldac( setup, ADC_GAIN_64XX );
		}
	}
}

static void prep_dac_caldacs_64xx( calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		if( channel == 0 )
		{
			reset_caldac( setup, DAC0_OFFSET_COARSE_64XX );
			reset_caldac( setup, DAC0_GAIN_COARSE_64XX );
			reset_caldac( setup, DAC0_OFFSET_FINE_64XX );
			reset_caldac( setup, DAC0_GAIN_FINE_64XX );
		}else
		{
			reset_caldac( setup, DAC1_OFFSET_COARSE_64XX );
			reset_caldac( setup, DAC1_GAIN_COARSE_64XX );
			reset_caldac( setup, DAC1_OFFSET_FINE_64XX );
			reset_caldac( setup, DAC1_GAIN_FINE_64XX );
		}
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			if( channel == 0 )
			{
				reset_caldac( setup, DAC0_OFFSET_COARSE_64XX );
				reset_caldac( setup, DAC0_GAIN_COARSE_64XX );
				reset_caldac( setup, DAC0_OFFSET_FINE_64XX );
				reset_caldac( setup, DAC0_GAIN_FINE_64XX );
			}else
			{
				reset_caldac( setup, DAC1_OFFSET_COARSE_64XX );
				reset_caldac( setup, DAC1_GAIN_COARSE_64XX );
				reset_caldac( setup, DAC1_OFFSET_FINE_64XX );
				reset_caldac( setup, DAC1_GAIN_FINE_64XX );
			}
		}
	}
}

static int cal_cb_pci_64xx( calibration_setup_t *setup )
{
	saved_calibration_t *saved_cals, *current_cal;
	int i, num_ai_ranges, num_ao_ranges, num_calibrations, retval;
	int adc_offset_for_ao = -1, adc_gain_for_ao = -1;
	int ai_range_for_ao;

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	if( num_ai_ranges < 1 ) return -1;
	if( setup->da_subdev >= 0 )
	{
		num_ao_ranges = comedi_get_n_ranges( setup->dev, setup->da_subdev, 0 );
		if( num_ai_ranges < 1 ) return -1;
	}else
		num_ao_ranges = 0;

	ai_range_for_ao = get_bipolar_lowgain( setup->dev, setup->ad_subdev );
	if( ai_range_for_ao < 0 ) return -1;

	num_calibrations = num_ai_ranges + 2 * num_ao_ranges;

	saved_cals = malloc( num_calibrations * sizeof( saved_calibration_t ) );
	if( saved_cals == NULL ) return -1;

	current_cal = saved_cals;
	for( i = 0; i < num_ai_ranges ; i++ )
	{
		prep_adc_caldacs_64xx( setup, i );

		cal_binary( setup, ai_ground_observable_index_64xx( i ), ADC_OFFSET_64XX );
		cal_binary( setup, ai_high_observable_index_64xx( i ), ADC_GAIN_64XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->ad_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_64XX ] );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		if( i == ai_range_for_ao )
		{
			adc_offset_for_ao = setup->caldacs[ ADC_OFFSET_64XX ].current;
			adc_gain_for_ao = setup->caldacs[ ADC_GAIN_64XX ].current;
		}
	}

	update_caldac( setup, ADC_OFFSET_64XX, adc_offset_for_ao );
	update_caldac( setup, ADC_GAIN_64XX, adc_gain_for_ao );
	for( i = 0; i < num_ao_ranges ; i++ )
	{
		prep_dac_caldacs_64xx( setup, 0, i );

		cal_binary( setup, ao_low_observable_index_64xx( setup, 0, i ), DAC0_OFFSET_COARSE_64XX );
		cal_binary( setup, ao_high_observable_index_64xx( setup, 0, i ), DAC0_GAIN_COARSE_64XX );
		cal_binary( setup, ao_low_observable_index_64xx( setup, 0, i ), DAC0_OFFSET_FINE_64XX );
		cal_binary( setup, ao_high_observable_index_64xx( setup, 0, i ), DAC0_GAIN_FINE_64XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET_COARSE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_GAIN_COARSE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET_FINE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_GAIN_FINE_64XX ] );
		sc_push_channel( current_cal, 0 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		prep_dac_caldacs_64xx( setup, 1, i );

		cal_binary( setup, ao_low_observable_index_64xx( setup, 1, i ), DAC1_OFFSET_COARSE_64XX );
		cal_binary( setup, ao_high_observable_index_64xx( setup, 1, i ), DAC1_GAIN_COARSE_64XX );
		cal_binary( setup, ao_low_observable_index_64xx( setup, 1, i ), DAC1_OFFSET_FINE_64XX );
		cal_binary( setup, ao_high_observable_index_64xx( setup, 1, i ), DAC1_GAIN_FINE_64XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_OFFSET_COARSE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_GAIN_COARSE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_OFFSET_FINE_64XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_GAIN_FINE_64XX ] );
		sc_push_channel( current_cal, 1 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

static void prep_adc_caldacs_60xx( calibration_setup_t *setup, unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		reset_caldac( setup, ADC_OFFSET_COARSE_60XX );
		reset_caldac( setup, ADC_OFFSET_FINE_60XX );
		reset_caldac( setup, ADC_GAIN_COARSE_60XX );
		reset_caldac( setup, ADC_GAIN_FINE_60XX );
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->ad_subdev,
			0, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			reset_caldac( setup, ADC_OFFSET_COARSE_60XX );
			reset_caldac( setup, ADC_OFFSET_FINE_60XX );
			reset_caldac( setup, ADC_GAIN_COARSE_60XX );
			reset_caldac( setup, ADC_GAIN_FINE_60XX );
		}
	}
}

static void prep_dac_caldacs_60xx( calibration_setup_t *setup,
	unsigned int channel, unsigned int range )
{
	int retval;

	if( setup->do_reset )
	{
		if( channel == 0 )
		{
			reset_caldac( setup, DAC0_OFFSET_60XX );
			reset_caldac( setup, DAC0_GAIN_60XX );
		}else
		{
			reset_caldac( setup, DAC1_OFFSET_60XX );
			reset_caldac( setup, DAC1_GAIN_60XX );
		}
	}else
	{
		retval = comedi_apply_calibration( setup->dev, setup->da_subdev,
			channel, range, AREF_GROUND, setup->cal_save_file_path);
		if( retval < 0 )
		{
			if( channel == 0 )
			{
				reset_caldac( setup, DAC0_OFFSET_60XX );
				reset_caldac( setup, DAC0_GAIN_60XX );
			}else
			{
				reset_caldac( setup, DAC1_OFFSET_60XX );
				reset_caldac( setup, DAC1_GAIN_60XX );
			}
		}
	}
}

static int cal_cb_pci_60xx( calibration_setup_t *setup )
{
	saved_calibration_t *saved_cals, *current_cal;
	int i, num_ai_ranges, num_ao_ranges, num_calibrations, retval;
	int adc_offset_fine_for_ao = -1, adc_offset_coarse_for_ao = -1,
		adc_gain_fine_for_ao = -1, adc_gain_coarse_for_ao = -1;
	static const int ai_range_for_ao = 0;

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

	current_cal = saved_cals;
	for( i = 0; i < num_ai_ranges ; i++ )
	{
		prep_adc_caldacs_60xx( setup, i );

		cal_binary( setup, ai_ground_observable_index_60xx( i ), ADC_OFFSET_COARSE_60XX );
		cal_binary( setup, ai_ground_observable_index_60xx( i ), ADC_OFFSET_FINE_60XX );
		cal_binary( setup, ai_high_observable_index_60xx( i ), ADC_GAIN_COARSE_60XX );
		cal_binary( setup, ai_high_observable_index_60xx( i ), ADC_GAIN_FINE_60XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->ad_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_FINE_60XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_COARSE_60XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_FINE_60XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_COARSE_60XX ] );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		if( i == ai_range_for_ao )
		{
			adc_offset_fine_for_ao = setup->caldacs[ ADC_OFFSET_FINE_60XX ].current;
			adc_offset_coarse_for_ao = setup->caldacs[ ADC_OFFSET_COARSE_60XX ].current;
			adc_gain_fine_for_ao = setup->caldacs[ ADC_GAIN_FINE_60XX ].current;
			adc_gain_coarse_for_ao = setup->caldacs[ ADC_GAIN_COARSE_60XX ].current;
		}
	}

	update_caldac( setup, ADC_OFFSET_FINE_60XX, adc_offset_fine_for_ao );
	update_caldac( setup, ADC_OFFSET_COARSE_60XX, adc_offset_coarse_for_ao );
	update_caldac( setup, ADC_GAIN_FINE_60XX, adc_gain_fine_for_ao );
	update_caldac( setup, ADC_GAIN_COARSE_60XX, adc_gain_coarse_for_ao );
	for( i = 0; i < num_ao_ranges ; i++ )
	{
		prep_dac_caldacs_60xx( setup, 0, i );

		cal_binary( setup, ao_low_observable_index_60xx( setup, 0, i ), DAC0_OFFSET_60XX );
		cal_binary( setup, ao_high_observable_index_60xx( setup, 0, i ), DAC0_GAIN_60XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET_60XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_GAIN_60XX ] );
		sc_push_channel( current_cal, 0 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		prep_dac_caldacs_60xx( setup, 1, i );

		cal_binary( setup, ao_low_observable_index_60xx( setup, 1, i ), DAC1_OFFSET_60XX );
		cal_binary( setup, ao_high_observable_index_60xx( setup, 1, i ), DAC1_GAIN_60XX );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_OFFSET_60XX ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_GAIN_60XX ] );
		sc_push_channel( current_cal, 1 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

static int cal_cb_pci_4020( calibration_setup_t *setup )
{
	enum cal_knobs_4020
	{
		ADC0_OFFSET = 0,
		ADC1_OFFSET,
		ADC2_OFFSET,
		ADC3_OFFSET,
		ADC0_GAIN,
		ADC1_GAIN,
		ADC2_GAIN,
		ADC3_GAIN,
	};

	cal1( setup, 0, ADC0_OFFSET );
	cal1_fine( setup, 0, ADC0_OFFSET );

	cal1( setup, 1, ADC1_OFFSET );
	cal1_fine( setup, 1, ADC1_OFFSET );

	cal1( setup, 2, ADC2_OFFSET );
	cal1_fine( setup, 2, ADC2_OFFSET );

	cal1( setup, 3, ADC3_OFFSET );
	cal1_fine( setup, 3, ADC3_OFFSET );

	cal_binary( setup, 4, ADC0_GAIN );
	cal1_fine( setup, 4, ADC0_GAIN );

	cal_binary( setup, 5, ADC1_GAIN );
	cal1_fine( setup, 5, ADC1_GAIN );

	cal_binary( setup, 6, ADC2_GAIN );
	cal1_fine( setup, 6, ADC2_GAIN );

	cal_binary( setup, 7, ADC3_GAIN );
	cal1_fine( setup, 7, ADC3_GAIN );

	return 0;
}

