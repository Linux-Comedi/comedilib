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
	lsampl_t cal_src_data[2];

	memset( &po_tmpl, 0, sizeof(po_tmpl) );
	po_tmpl.insn = INSN_CONFIG;
	po_tmpl.n = 2;
	po_tmpl.subdev = ai_subdev;
	po_tmpl.data = cal_src_data;
	cal_src_data[0] = INSN_CONFIG_ALT_SOURCE;

	memset( &tmpl, 0, sizeof(tmpl) );
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = ai_subdev;

	o = setup->observables + 0;
	o->name = "ground calibration source, 10V bipolar range, ground reference";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data[1] = 0;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK_FLAGS( 0, 0, AREF_GROUND, CR_ALT_SOURCE);
	o->target = 0.0;

	o = setup->observables + 1;
	o->name = "5V calibration source, 10V bipolar range, ground reference";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.data[1] = 2;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK_FLAGS( 0, 0, AREF_GROUND, CR_ALT_SOURCE);
	o->target = 5.0;

	setup->n_observables = 2;

	return 0;
}

int init_observables_4020( calibration_setup_t *setup )
{
	return 0;
}

int cal_cb_pci_64xx( calibration_setup_t *setup )
{
	return 0;
}

int cal_cb_pci_60xx( calibration_setup_t *setup )
{
	return 0;
}

int cal_cb_pci_4020( calibration_setup_t *setup )
{
	return 0;
}

