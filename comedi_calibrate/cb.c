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

#include "calib.h"


char cb_id[] = "$Id$";

struct board_struct{
	char *name;
	int status;
	int (*setup)( calibration_setup_t *setup );
};

int cb_setup_board(void);
void cb_setup_observables(void);

int setup_cb_pci_64xx( calibration_setup_t *setup );
int setup_cb_pci_60xx( calibration_setup_t *setup );
int setup_cb_pci_4020( calibration_setup_t *setup );

int cal_cb_pci_64xx( calibration_setup_t *setup );
int cal_cb_pci_60xx( calibration_setup_t *setup );
int cal_cb_pci_4020( calibration_setup_t *setup );

int init_observables_64xx( calibration_setup_t *setup );
int init_observables_60xx( calibration_setup_t *setup );
int init_observables_4020( calibration_setup_t *setup );

static struct board_struct boards[]={
	{ "pci-das6402/16",	STATUS_DONE,	setup_cb_pci_64xx },
	{ "pci-das6402/12",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m1/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m2/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das64/m3/16",	STATUS_GUESS,	setup_cb_pci_64xx },
	{ "pci-das6025",	STATUS_DONE,	setup_cb_pci_60xx },
	{ "pci-das6034",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das6035",	STATUS_GUESS,	setup_cb_pci_60xx },
	{ "pci-das4020/12",	STATUS_DONE,	setup_cb_pci_4020 },
};

static const int num_boards = ( sizeof(boards) / sizeof(boards[0]) );

enum {
	cb_zero_offset_low = 0,
	cb_zero_offset_high,
	cb_reference_low,
	cb_unip_offset_low,
	cb_ao0_zero_offset,
	cb_ao0_reference,
	cb_ao1_zero_offset,
	cb_ao1_reference,
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
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

int init_observables_64xx( calibration_setup_t *setup )
{
	return 0;
}

int init_observables_60xx( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	static const int ai_subdev = 0;

	memset( &po_tmpl, 0, sizeof(po_tmpl) );
	po_tmpl.insn = INSN_CONFIG;
	po_tmpl.n = 2;
	po_tmpl.subdev = ai_subdev;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = ai_subdev;

	o = setup->observables + 0;
	o->name = "ground calibration source, 10V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 0;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND);
	o->target = 0.0;

	o = setup->observables + 1;
	o->name = "5V calibration source, 10V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 2;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND);
	o->target = 5.0;

	setup->n_observables = 2;

	return 0;
}

int init_observables_4020( calibration_setup_t *setup )
{
	comedi_insn tmpl, po_tmpl;
	observable *o;
	static const int ai_subdev = 0;

	memset( &po_tmpl, 0, sizeof(po_tmpl) );
	po_tmpl.insn = INSN_CONFIG;
	po_tmpl.n = 2;
	po_tmpl.subdev = ai_subdev;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = ai_subdev;

	o = setup->observables + 0;
	o->name = "ground calibration source, ch 0, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 7;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND);
	o->target = 0.0;

	o = setup->observables + 1;
	o->name = "ground calibration source, ch 1, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 7;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND);
	o->target = 0.0;

	o = setup->observables + 2;
	o->name = "ground calibration source, ch 2, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 7;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 2, 0, AREF_GROUND);
	o->target = 0.0;

	o = setup->observables + 3;
	o->name = "ground calibration source, ch 3, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 7;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 3, 0, AREF_GROUND);
	o->target = 0.0;

	o = setup->observables + 4;
	o->name = "4.375V calibration source, ch 0, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 5;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND);
	o->target = 4.375;

	o = setup->observables + 5;
	o->name = "4.375V calibration source, ch 1, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 5;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND);
	o->target = 4.375;

	o = setup->observables + 6;
	o->name = "4.375V calibration source, ch 2, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 5;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 2, 0, AREF_GROUND);
	o->target = 4.375;

	o = setup->observables + 7;
	o->name = "4.375V calibration source, ch 3, 5V bipolar range, ground referenced";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data = o->preobserve_data;
	o->preobserve_insn.data[0] = INSN_CONFIG_ALT_SOURCE;
	o->preobserve_insn.data[1] = 5;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 3, 0, AREF_GROUND);
	o->target = 4.375;

	setup->n_observables = 8;

	return 0;
}

int cal_cb_pci_64xx( calibration_setup_t *setup )
{
	return 0;
}

int cal_cb_pci_60xx( calibration_setup_t *setup )
{
	enum caldacs_60xx
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

	cal1( setup, 0, ADC_OFFSET_COARSE );
	cal1_fine( setup, 0, ADC_OFFSET_COARSE );

	cal1( setup, 0, ADC_OFFSET_FINE );
	cal1_fine( setup, 0, ADC_OFFSET_FINE );

	cal1( setup, 1, ADC_GAIN_COARSE );
	cal1_fine( setup, 1, ADC_GAIN_COARSE );

	cal1( setup, 1, ADC_GAIN_FINE );
	cal1_fine( setup, 1, ADC_GAIN_FINE );

	return 0;
}

int cal_cb_pci_4020( calibration_setup_t *setup )
{
	enum caldacs_4020
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

	cal1( setup, 4, ADC0_GAIN );
	cal1_fine( setup, 4, ADC0_GAIN );

	cal1( setup, 5, ADC1_GAIN );
	cal1_fine( setup, 5, ADC1_GAIN );

	cal1( setup, 6, ADC2_GAIN );
	cal1_fine( setup, 6, ADC2_GAIN );

	cal1( setup, 7, ADC3_GAIN );
	cal1_fine( setup, 7, ADC3_GAIN );

	return 0;
}

