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
/*
	TODO:
	read calibration voltage targets from eeprom
	calibrate all possible ranges and save to file
*/

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
int setup_cb_pci_1602_16( calibration_setup_t *setup );

int cal_cb_pci_64xx( calibration_setup_t *setup );
int cal_cb_pci_60xx( calibration_setup_t *setup );
int cal_cb_pci_4020( calibration_setup_t *setup );
int cal_cb_pci_1xxx( calibration_setup_t *setup );
int cal_cb_pci_1602_16( calibration_setup_t *setup );

int init_observables_64xx( calibration_setup_t *setup );
int init_observables_60xx( calibration_setup_t *setup );
int init_observables_4020( calibration_setup_t *setup );
int init_observables_1xxx( calibration_setup_t *setup );
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
	{ "pci-das1001",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1002",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1200",	STATUS_DONE,	setup_cb_pci_1xxx },
	{ "pci-das1200/jr",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/12",	STATUS_GUESS,	setup_cb_pci_1xxx },
	{ "pci-das1602/16",	STATUS_GUESS,	setup_cb_pci_1602_16 },
	{ "pci-das1602/16/jr",	STATUS_GUESS,	setup_cb_pci_1602_16 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum observables_60xx {
	OBS_0V_RANGE_10V_BIP_60XX = 0,
	OBS_5V_RANGE_10V_BIP_60XX,
};

enum observables_64xx {
	OBS_0V_RANGE_10V_BIP_64XX = 0,
	OBS_7V_RANGE_10V_BIP_64XX,
};

enum observables_1xxx {
	OBS_0V_RANGE_10V_BIP_1XXX = 0,
	OBS_7V_RANGE_10V_BIP_1XXX,
};

enum observables_1602_16 {
	OBS_0V_RANGE_10V_BIP_1602_16 = 0,
	OBS_7V_RANGE_10V_BIP_1602_16,
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

int init_observables_60xx( calibration_setup_t *setup )
{
	comedi_insn tmpl;//, po_tmpl;
	observable *o;
	int retval;
	float target;
	enum source_eeprom_addr
	{
		EEPROM_10V_CHAN = 0x30,
		EEPROM_5V_CHAN = 0x32,
		EEPROM_500mV_CHAN = 0x38,
		EEPROM_50mV_CHAN = 0x3e,
		EEPROM_8mV_CHAN = 0x40,
	};
	enum calibration_source
	{
		CAL_SRC_GROUND = 0,
		CAL_SRC_10V = 1,
		CAL_SRC_5V = 2,
		CAL_SRC_500mV = 3,
		CAL_SRC_50mV = 4,
		CAL_SRC_5mV = 5,	// XXX check
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

	o = setup->observables + OBS_0V_RANGE_10V_BIP_60XX;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + OBS_5V_RANGE_10V_BIP_60XX;
	o->name = "5V calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_5V;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 5.0;
	retval = actual_source_voltage( setup->dev, setup->eeprom_subdev, EEPROM_5V_CHAN, &target );
	if( retval == 0 )
		o->target = target;

	setup->n_observables = 2;

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

	o = setup->observables + OBS_0V_RANGE_10V_BIP_1XXX;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->reference_source = CAL_SRC_GROUND;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND) | CR_ALT_SOURCE | CR_ALT_FILTER;
	o->target = 0.0;

	o = setup->observables + OBS_7V_RANGE_10V_BIP_1XXX;
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

	cal1( setup, OBS_0V_RANGE_10V_BIP_60XX, ADC_OFFSET_COARSE );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_60XX, ADC_OFFSET_COARSE );

	cal1( setup, OBS_0V_RANGE_10V_BIP_60XX, ADC_OFFSET_FINE );
	cal1_fine( setup, OBS_0V_RANGE_10V_BIP_60XX, ADC_OFFSET_FINE );

	cal1( setup, OBS_5V_RANGE_10V_BIP_60XX, ADC_GAIN_COARSE );
	cal1_fine( setup, OBS_5V_RANGE_10V_BIP_60XX, ADC_GAIN_COARSE );

	cal1( setup, OBS_5V_RANGE_10V_BIP_60XX, ADC_GAIN_FINE );
	cal1_fine( setup, OBS_5V_RANGE_10V_BIP_60XX, ADC_GAIN_FINE );

	return 0;
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
float eeprom_to_source(uint16_t ls_word, uint16_t ms_word)
{
	union translator
	{
		uint32_t bits;
		float	value;
	};

	union translator my_translator;

	my_translator.bits = ( ls_word & 0xffff ) | ( ( ms_word << 16 ) & 0xffff0000 );

	return my_translator.value;
}

int actual_source_voltage( comedi_t *dev, unsigned int subdevice, unsigned int eeprom_channel, float *voltage)
{
	int retval;
	uint16_t word[2];
	unsigned int i;
	lsampl_t data;

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

	*voltage = eeprom_to_source( word[0], word[1] );
	DPRINT(0, "eeprom ch %i,%i give calibration source of %gV\n", eeprom_channel, eeprom_channel + 1, *voltage);
	return 0;
}
