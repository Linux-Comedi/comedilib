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
	int (*setup)( calibration_setup *setup );
};

int cb_setup_board(void);
void cb_setup_observables(void);

int setup_cb_pci_64xx( calibration_setup *setup );
int setup_cb_pci_60xx( calibration_setup *setup );
int setup_cb_pci_4020( calibration_setup *setup );

int cal_cb_pci_64xx( calibration_setup *setup );
int cal_cb_pci_60xx( calibration_setup *setup );
int cal_cb_pci_4020( calibration_setup *setup );

int init_observables_64xx( calibration_setup *setup );
int init_observables_60xx( calibration_setup *setup );
int init_observables_4020( calibration_setup *setup );

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

int cb_setup( calibration_setup *setup, const char *device_name )
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

int setup_cb_pci_64xx( calibration_setup *setup )
{
	init_observables_64xx( setup );
	setup->do_cal = cal_cb_pci_64xx;
	return 0;
}

int setup_cb_pci_60xx( calibration_setup *setup )
{
	init_observables_60xx( setup );
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

int setup_cb_pci_4020( calibration_setup *setup )
{
	init_observables_4020( setup );
	setup->do_cal = cal_cb_pci_60xx;
	return 0;
}

int init_observables_64xx( calibration_setup *setup )
{
	return 0;
}

int init_observables_60xx( calibration_setup *setup )
{
	return 0;
}

int init_observables_4020( calibration_setup *setup )
{
	return 0;
}

int cal_cb_pci_64xx( calibration_setup *setup )
{
	return 0;
}

int cal_cb_pci_60xx( calibration_setup *setup )
{
	return 0;
}

int cal_cb_pci_4020( calibration_setup *setup )
{
	return 0;
}

