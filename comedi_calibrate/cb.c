/***************************************************************************
	cb.c  -  calibration support for some Measurement computing boards.
	Based on ni.c by David Schleef.
                             -------------------

    begin                : Sat Apr 27 2002
    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

int setup_cb_pci_64xx( calibration_setup_t *setup );
int setup_cb_pci_60xx( calibration_setup_t *setup );
int setup_cb_pci_4020( calibration_setup_t *setup );
int setup_cb_pci_1xxx( calibration_setup_t *setup );
int setup_cb_pci_1001( calibration_setup_t *setup );
int setup_cb_pci_1602_16( calibration_setup_t *setup );

int cal_cb_pci_64xx( calibration_setup_t *setup );
int cal_cb_pci_60xx( calibration_setup_t *setup );
int cal_cb_pci_4020( calibration_setup_t *setup );
int cal_cb_pci_1xxx( calibration_setup_t *setup );
int cal_cb_pci_1001( calibration_setup_t *setup );
int cal_cb_pci_1602_16( calibration_setup_t *setup );

int init_observables_64xx( calibration_setup_t *setup );
int init_observables_60xx( calibration_setup_t *setup );
int init_observables_4020( calibration_setup_t *setup );
int init_observables_1xxx( calibration_setup_t *setup );
int init_observables_1001( calibration_setup_t *setup );
int init_observables_1602_16( calibration_setup_t *setup );

int actual_source_voltage( comedi_t *dev, unsigned int subdevice, unsigned int eeprom_channel, float *voltage);

static struct board_struct boards[]={
	{ "pci-das6402/16",	STATUS_DONE,	setup_cb_pci_64xx },
	{ "pci-das6402/12",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m1/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m2/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m3/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das6023",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6025",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6034",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6035",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das4020/12",	STATUS_DONE,	setup_cb_pci_4020 },
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

enum observables_64xx {
	OBS_0V_RANGE_10V_BIP_64XX = 0,
	OBS_7V_RANGE_10V_BIP_64XX,
};

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

int setup_cb_pci_64xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;
	static const int calpot_subdev = 7;

	init_observables_64xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_64xx;
	return 0;
}

int setup_cb_pci_60xx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;

	init_observables_60xx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

int setup_cb_pci_4020( calibration_setup_t *setup )
{
	static const int caldac_subdev = 6;

	init_observables_4020( setup );
	setup_caldacs( setup, caldac_subdev );
	setup->do_cal = cal_cb_pci_4020;
	return 0;
}

int setup_cb_pci_1xxx( calibration_setup_t *setup )
{
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;

	init_observables_1xxx( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_1xxx;
	return 0;
}

int setup_cb_pci_1001( calibration_setup_t *setup )
{
	static const int caldac_subdev = 4;
	static const int calpot_subdev = 5;

	init_observables_1001( setup );
	setup_caldacs( setup, caldac_subdev );
	setup_caldacs( setup, calpot_subdev );
	setup->do_cal = cal_cb_pci_1001;
	return 0;
}

int setup_cb_pci_1602_16( calibration_setup_t *setup )
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

int init_observables_64xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;//, po_tmpl;
	observable *o;
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
	enum calibration_source
	{
		CAL_SRC_GROUND = 0,
		CAL_SRC_7V = 1,
		CAL_SRC_3500mV = 2,
		CAL_SRC_1750mV = 3,
		CAL_SRC_875mV = 4,
		CAL_SRC_8600uV = 5,
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

	o = setup->observables + OBS_0V_RANGE_10V_BIP_64XX;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + OBS_7V_RANGE_10V_BIP_64XX;
	o->name = "7V calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_7V;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 7.0;
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	setup->n_observables = 2;

	return 0;
}

unsigned int ai_high_observable_index_60xx( unsigned int ai_range )
{
	return ai_range * 2 + 1;
}

unsigned int ai_ground_observable_index_60xx( unsigned int ai_range )
{
	return ai_range * 2;
}

unsigned int ao_high_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range + 1;
}

unsigned int ao_low_observable_index_60xx( const calibration_setup_t *setup,
	unsigned int channel, unsigned int ao_range )
{
	int num_ai_ranges;

	num_ai_ranges = comedi_get_n_ranges( setup->dev, setup->ad_subdev, 0 );
	assert( num_ai_ranges >= 0 );

	return 2 * num_ai_ranges + 2 * channel + 4 * ao_range;
}

int ai_cal_src_voltage_60xx( calibration_setup_t *setup,
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
			retval = actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_10V_CHAN, voltage );
			break;
		case CS_60XX_5V:
			retval = actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_5V_CHAN, voltage );
			break;
		case CS_60XX_500mV:
			retval = actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_500mV_CHAN, voltage );
			break;
		case CS_60XX_50mV:
			retval = actual_source_voltage( setup->dev, setup->eeprom_subdev,
				EEPROM_50mV_CHAN, voltage );
			break;
		default:
			fprintf( stderr, "invalid calibration_source in ai_cal_src_voltage_60xx()\n" );
			retval = -1;
			break;
	}
	return retval;
}

int high_ai_cal_src_60xx( calibration_setup_t *setup, unsigned int ai_range )
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

int ao_cal_src_60xx( unsigned int channel )
{
	switch( channel )
	{
	case 0:
		return 6;
		break;
	case 1:
		return 7;
		break;
	default:
		return -1;
		break;
	}
}

int init_observables_60xx( calibration_setup_t *setup )
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
		o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
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
		o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
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

int init_observables_4020( calibration_setup_t *setup )
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
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 5;
	o->name = "4.375V calibration source, ch 1, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 6;
	o->name = "4.375V calibration source, ch 2, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 2, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	o = setup->observables + 7;
	o->name = "4.375V calibration source, ch 3, 5V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_4375mV;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 3, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 4.375;
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_4375mV_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	setup->n_observables = 8;

	return 0;
}

int init_observables_1xxx( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	int retval;
	float target;
	enum source_eeprom_addr
	{
		EEPROM_7V_CHAN = 0x80,
		EEPROM_3500mV_CHAN = 0x84,
		EEPROM_1750mV_CHAN = 0x88,
		EEPROM_875mV_CHAN = 0x8c,
		EEPROM_8600uV_CHAN = 0x90,
	};
	enum calibration_source
	{
		CAL_SRC_GROUND = 0,
		CAL_SRC_7V = 1,
		CAL_SRC_3500mV = 2,
		CAL_SRC_1750mV = 3,
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
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
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

int init_observables_1001( calibration_setup_t *setup )
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
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
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

int init_observables_1602_16( calibration_setup_t *setup )
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
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_7V_CHAN, &target );
	if( retval == 0 )
		o->target = target;
#endif
	setup->n_observables = 2;

	return 0;
}

int cal_cb_pci_64xx( calibration_setup_t *setup )
{
	enum cal_knobs_64xx
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
		ADC_OFFSET,
	};

	cal1( setup, OBS_0V_RANGE_10V_BIP_64XX, ADC_OFFSET );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_64XX, ADC_OFFSET );

	cal1( setup, OBS_7V_RANGE_10V_BIP_64XX, ADC_GAIN );
	cal1_fine( setup, OBS_7V_RANGE_10V_BIP_64XX, ADC_GAIN );

	return 0;
}

int cal_cb_pci_60xx( calibration_setup_t *setup )
{
	saved_calibration_t *saved_cals, *current_cal;
	enum cal_knobs_60xx
	{
		DAC0_OFFSET = 0,
		DAC0_GAIN,
		DAC1_OFFSET,
		DAC1_GAIN,
		ADC_OFFSET_FINE,
		ADC_OFFSET_COARSE,
		ADC_GAIN_COARSE,
		ADC_GAIN_FINE,
	};
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
		reset_caldac( setup, ADC_OFFSET_COARSE );
		reset_caldac( setup, ADC_OFFSET_FINE );
		reset_caldac( setup, ADC_GAIN_COARSE );
		reset_caldac( setup, ADC_GAIN_FINE );

		cal_binary( setup, ai_ground_observable_index_60xx( i ), ADC_OFFSET_COARSE );
		cal_binary( setup, ai_ground_observable_index_60xx( i ), ADC_OFFSET_FINE );
		cal_binary( setup, ai_high_observable_index_60xx( i ), ADC_GAIN_COARSE );
		cal_binary( setup, ai_high_observable_index_60xx( i ), ADC_GAIN_FINE );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->ad_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_FINE ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_OFFSET_COARSE ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_FINE ] );
		sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN_COARSE ] );
		sc_push_channel( current_cal, SC_ALL_CHANNELS );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		if( i == ai_range_for_ao )
		{
			adc_offset_fine_for_ao = setup->caldacs[ ADC_OFFSET_FINE ].current;
			adc_offset_coarse_for_ao = setup->caldacs[ ADC_OFFSET_COARSE ].current;
			adc_gain_fine_for_ao = setup->caldacs[ ADC_GAIN_FINE ].current;
			adc_gain_coarse_for_ao = setup->caldacs[ ADC_GAIN_COARSE ].current;
		}
	}

	update_caldac( setup, ADC_OFFSET_FINE, adc_offset_fine_for_ao );
	update_caldac( setup, ADC_OFFSET_COARSE, adc_offset_coarse_for_ao );
	update_caldac( setup, ADC_GAIN_FINE, adc_gain_fine_for_ao );
	update_caldac( setup, ADC_GAIN_COARSE, adc_gain_coarse_for_ao );
	for( i = 0; i < num_ao_ranges ; i++ )
	{
		reset_caldac( setup, DAC0_OFFSET );
		reset_caldac( setup, DAC0_GAIN );

		cal_binary( setup, ao_low_observable_index_60xx( setup, 0, i ), DAC0_OFFSET );
		cal_binary( setup, ao_high_observable_index_60xx( setup, 0, i ), DAC0_GAIN );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_GAIN ] );
		sc_push_channel( current_cal, 0 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;

		reset_caldac( setup, DAC1_OFFSET );
		reset_caldac( setup, DAC1_GAIN );

		cal_binary( setup, ao_low_observable_index_60xx( setup, 1, i ), DAC1_OFFSET );
		cal_binary( setup, ao_high_observable_index_60xx( setup, 1, i ), DAC1_GAIN );

		memset( current_cal, 0, sizeof( saved_calibration_t ) );
		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_OFFSET ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_GAIN ] );
		sc_push_channel( current_cal, 1 );
		sc_push_range( current_cal, i );
		sc_push_aref( current_cal, SC_ALL_AREFS );

		current_cal++;
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations);
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );
	free( saved_cals );
	return retval;
}

int cal_cb_pci_4020( calibration_setup_t *setup )
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

int cal_cb_pci_1xxx( calibration_setup_t *setup )
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

int cal_cb_pci_1001( calibration_setup_t *setup )
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

int cal_cb_pci_1602_16( calibration_setup_t *setup )
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
float eeprom16_to_source( uint16_t *data )
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

float eeprom8_to_source( uint8_t *data )
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

int actual_source_voltage( comedi_t *dev, unsigned int subdevice, unsigned int eeprom_channel, float *voltage)
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
				perror( "actual_source_voltage()" );
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
				perror( "actual_source_voltage()" );
				return retval;
			}
			byte[ i ] = data;
		}
		*voltage = eeprom8_to_source( byte );
	}else
	{
		fprintf( stderr, "actual_source_voltage(), maxdata = 0x%x invalid\n", max_data );
		return -1;
	}

	DPRINT(0, "eeprom ch 0x%x gives calibration source of %gV\n", eeprom_channel, *voltage);
	return 0;
}
