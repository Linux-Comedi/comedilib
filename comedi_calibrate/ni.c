/*
   A little auto-calibration utility, for boards
   that support it.

   Right now, it only supports NI E series boards,
   but it should be easily portable.

   A few things need improvement here:
    - current system gets "close", but doesn't
      do any fine-tuning
    - no pre/post gain discrimination for the
      A/D zero offset.
    - should read (and use) the actual reference
      voltage value from eeprom
    - statistics would be nice, to show how good
      the calibration is.
    - doesn't check unipolar ranges
    - "alternate calibrations" would be cool--to
      accurately measure 0 in a unipolar range
    - more portable
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
#include "comedilib.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "calib.h"


char ni_id[] = "$Id$";

struct board_struct{
	char *name;
	int status;
	int (*cal)( calibration_setup_t *setup);
	void (*setup_observables)( calibration_setup_t *setup );
};

static int ni_setup_board( calibration_setup_t *setup , const char *device_name );
static void ni_setup_observables( calibration_setup_t *setup );
static void ni_setup_observables_611x( calibration_setup_t *setup );

static int cal_ni_at_mio_16e_2(calibration_setup_t *setup);
static int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup);
static int cal_ni_at_mio_16e_1(calibration_setup_t *setup);
static int cal_ni_pci_mio_16e_1(calibration_setup_t *setup);
static int cal_ni_pci_6025e(calibration_setup_t *setup);
static int cal_ni_pci_6035e(calibration_setup_t *setup);
static int cal_ni_pci_6071e(calibration_setup_t *setup);
static int cal_ni_pxi_6071e(calibration_setup_t *setup);
static int cal_ni_at_mio_16e_10(calibration_setup_t *setup);
static int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup);
static int cal_ni_pci_6023e(calibration_setup_t *setup);
static int cal_ni_pci_6024e(calibration_setup_t *setup);
static int cal_ni_at_mio_16xe_50(calibration_setup_t *setup);
static int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup);
static int cal_ni_pci_6052e(calibration_setup_t *setup);
static int cal_ni_pci_mio_16e_4(calibration_setup_t *setup);
static int cal_ni_pci_6032e(calibration_setup_t *setup);
static int cal_ni_daqcard_ai_16e_4(calibration_setup_t *setup);
static int cal_ni_pci_611x(calibration_setup_t *setup);
static int cal_ni_daqcard_6062e(calibration_setup_t *setup);

static double ni_get_reference( calibration_setup_t *setup, int lsb_loc,int msb_loc);

static struct board_struct boards[]={
	{ "at-mio-16e-2",	STATUS_DONE,	cal_ni_at_mio_16e_2,	ni_setup_observables  },
	{ "DAQCard-ai-16xe-50",	STATUS_DONE,	cal_ni_daqcard_ai_16xe_50,	ni_setup_observables },
	{ "at-mio-16xe-50",	STATUS_SOME,	cal_ni_at_mio_16xe_50,	ni_setup_observables },
	{ "at-mio-16e-1",	STATUS_SOME,	cal_ni_at_mio_16e_1,	ni_setup_observables },
	{ "pci-mio-16e-1",	STATUS_DONE,	cal_ni_pci_mio_16e_1,	ni_setup_observables },
	{ "pci-6025e",		STATUS_SOME,	cal_ni_pci_6025e,	ni_setup_observables },
	{ "pci-6035e",		STATUS_DONE,	cal_ni_pci_6035e,	ni_setup_observables },
	{ "pci-6071e",		STATUS_SOME,	cal_ni_pci_6071e,	ni_setup_observables },
	{ "pxi-6071e",		STATUS_GUESS,	cal_ni_pxi_6071e,	ni_setup_observables },
	{ "at-mio-16e-10",	STATUS_GUESS,	cal_ni_at_mio_16e_10,	ni_setup_observables },
	{ "pci-mio-16xe-50",	STATUS_SOME,	cal_ni_pci_mio_16xe_50,	ni_setup_observables },
	{ "pci-6023e",		STATUS_DONE,	cal_ni_pci_6023e,	ni_setup_observables },
	{ "pci-mio-16xe-10",	STATUS_DONE,	cal_ni_pci_mio_16xe_10,	ni_setup_observables },
	{ "pci-6052e",		STATUS_DONE,	cal_ni_pci_6052e,	ni_setup_observables },
	{ "pci-6024e",		STATUS_SOME,	cal_ni_pci_6024e,	ni_setup_observables },
	{ "pci-mio-16e-4",	STATUS_SOME,    cal_ni_pci_mio_16e_4,	ni_setup_observables },
	{ "pci-6032e",		STATUS_DONE,	cal_ni_pci_6032e,	ni_setup_observables },
	{ "DAQCard-ai-16e-4",	STATUS_DONE,	cal_ni_daqcard_ai_16e_4,	ni_setup_observables },
	{ "pci-6110",	STATUS_DONE,	cal_ni_pci_611x,	ni_setup_observables_611x },
	{ "pci-6111",	STATUS_DONE,	cal_ni_pci_611x,	ni_setup_observables_611x },
	{ "DAQCard-6062E", STATUS_SOME, cal_ni_daqcard_6062e, ni_setup_observables },
	{ "DAQCard-6024E",	STATUS_UNKNOWN, NULL, ni_setup_observables },
#if 0
//	{ "at-mio-16de-10",	cal_ni_unknown },
	{ "at-mio-64e-3",	cal_ni_16e_1 },
//	{ "at-mio-16xe-50",	cal_ni_unknown },
//	{ "at-mio-16xe-10",	cal_ni_unknown },
//	{ "at-ai-16xe-10",	cal_ni_unknown },
//	{ "pxi-6030e",		cal_ni_unknown },
//	{ "pxi-6040e",		cal_ni_unknown },
//	{ "pci-6031e",		cal_ni_unknown },
//	{ "pci-6033e",		cal_ni_unknown },
	{ "pxi-6025e",		cal_ni_6023e }, // guess
	{ "pci-6034e",		cal_ni_6023e }, // guess
//	{ "pci-6711",		cal_ni_unknown },
//	{ "pci-6713",		cal_ni_unknown },
//	{ "pxi-6070e",		cal_ni_unknown },
//	{ "pxi-6052e",		cal_ni_unknown },
#endif
};
#define n_boards (sizeof(boards)/sizeof(boards[0]))

enum observables{
	ni_zero_offset_low = 0,
	ni_zero_offset_high,
	ni_reference_low,
	ni_unip_zero_offset_low,
	ni_unip_zero_offset_high,
	ni_unip_reference_low,
	ni_ao0_zero_offset,
	ni_ao0_reference,
	ni_ao1_zero_offset,
	ni_ao1_reference,
};

enum observables_611x{
	ni_ao0_zero_offset_611x = 0,
	ni_ao0_reference_611x = 1,
	ni_ao1_zero_offset_611x = 2,
	ni_ao1_reference_611x = 3,
};
inline static int ni_zero_offset_611x( int channel ) {
	return 4 + 2 * channel;
};
inline static int ni_reference_611x( int channel ) {
	return 5 + 2 * channel;
};

enum reference_sources {
	REF_GND_GND = 0,
	REF_AOGND_AIGND = 1,
	REF_DAC0_GND = 2,
	REF_DAC1_GND = 3,
	REF_CALSRC_CALSRC = 4,
	REF_CALSRC_GND = 5,
	REF_DAC0_CALSRC = 6,
	REF_DAC1_CALSRC = 7,
};

int ni_setup( calibration_setup_t *setup , const char *device_name )
{
	int retval;

	retval = ni_setup_board( setup, device_name );
	if( retval < 0 ) return retval;
	setup_caldacs( setup, setup->caldac_subdev );

	return 0;
}

static int ni_setup_board( calibration_setup_t *setup, const char *device_name )
{
	int i;

	for(i = 0; i < n_boards; i++ ){
		if(!strcmp( device_name, boards[i].name )){
			setup->status = boards[i].status;
			setup->do_cal = boards[i].cal;
			boards[i].setup_observables( setup );
			break;
		}
	}
	if( i == n_boards ) return -1;
	return 0;
}

static void ni_setup_observables( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	int unipolar_highgain;
	double voltage_reference;
	observable *o;

	bipolar_lowgain = get_bipolar_lowgain( setup->dev, setup->ad_subdev);
	bipolar_highgain = get_bipolar_highgain( setup->dev, setup->ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain( setup->dev, setup->ad_subdev);
	unipolar_highgain = get_unipolar_highgain( setup->dev, setup->ad_subdev);

	voltage_reference = 5.000;

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	setup->n_observables = 10;

	/* 0 offset, low gain */
	o = setup->observables + ni_zero_offset_low;
	o->name = "ai, bipolar zero offset, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_GND_GND,bipolar_lowgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_GND_GND;
	o->target = 0;

	/* 0 offset, high gain */
	o = setup->observables + ni_zero_offset_high;
	o->name = "ai, bipolar zero offset, high gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_GND_GND,bipolar_highgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_GND_GND;
	o->target = 0;

	/* voltage reference */
	o = setup->observables + ni_reference_low;
	o->name = "ai, bipolar voltage reference, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(REF_CALSRC_GND,bipolar_lowgain,AREF_OTHER)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_CALSRC_GND;
	o->target = voltage_reference;

	if(unipolar_lowgain>=0){
		o = setup->observables + ni_unip_zero_offset_low;
		o->name = "ai, unipolar zero offset, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_GND_GND,unipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_GND_GND;
		o->target = 0.0;

		o = setup->observables + ni_unip_reference_low;
		o->name = "ai, unipolar voltage reference, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_CALSRC_GND,unipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_CALSRC_GND;
		o->target = voltage_reference;
	}

	if(unipolar_highgain >= 0)
	{
		o = setup->observables + ni_unip_zero_offset_high;
		o->name = "ai, unipolar zero offset, high gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_GND_GND,unipolar_highgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_GND_GND;
		o->target = 0.0;
	}

	if(setup->da_subdev>=0){
		comedi_insn po_tmpl;

		memset(&po_tmpl,0,sizeof(po_tmpl));
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = setup->da_subdev;

		/* ao 0, zero offset */
		o = setup->observables + ni_ao0_zero_offset;
		o->name = "ao 0, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC0_GND,bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC0_GND;
		set_target( setup, ni_ao0_zero_offset,0.0);

		/* ao 0, gain */
		o = setup->observables + ni_ao0_reference;
		o->name = "ao 0, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC0_CALSRC,bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC0_CALSRC;
		set_target( setup, ni_ao0_reference,5.0);
		o->target -= voltage_reference;

		/* ao 1, zero offset */
		o = setup->observables + ni_ao1_zero_offset;
		o->name = "ao 1, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC1_GND,bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC1_GND;
		set_target( setup, ni_ao1_zero_offset,0.0);

		/* ao 1, gain */
		o = setup->observables + ni_ao1_reference;
		o->name = "ao 1, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(REF_DAC1_CALSRC,bipolar_lowgain,AREF_OTHER)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_DAC1_CALSRC;
		set_target( setup, ni_ao1_reference,5.0);
		o->target -= voltage_reference;
	}
}

/* XXX for +-50V and +-20V ranges, the reference source goes 0V
 * to 50V instead of 0V to 5V */
static unsigned int cal_gain_register_bits_611x( double *voltage )
{
	unsigned int bits;

	bits = 200.0 * ( *voltage / 5.0 );
	if( bits > 200 ) bits = 200;

	*voltage = 5.0 * ( bits / 200.0 );
	return bits;
}

static unsigned int ref_source_611x( unsigned int ref_source, unsigned int cal_gain_bits )
{
	return ( ref_source & 0xf ) | ( ( cal_gain_bits << 4 ) & 0xff0 );
}

static void ni_setup_observables_611x( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	comedi_insn po_tmpl;
	int range, ai_range_for_ao;
	double voltage_reference;
	observable *o;
	int ai_chan;
	int num_chans;
	int cal_gain_reg_bits;

	setup->settling_time_ns = 1000000;

	range = 2;

/*	voltage_reference = ni_get_reference( setup, 468, 469 ); */
	voltage_reference = 5.0;
	cal_gain_reg_bits = cal_gain_register_bits_611x( &voltage_reference );

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = setup->ad_subdev;

	num_chans = comedi_get_n_channels( setup->dev, setup->ad_subdev );

	for( ai_chan = 0; ai_chan < num_chans; ai_chan++ )
	{
		/* 0 offset */
		o = setup->observables + ni_zero_offset_611x( ai_chan );
		o->name = "ai, bipolar zero offset";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK(ai_chan, range, AREF_DIFF)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = REF_GND_GND;
		o->target = 0.0;

		/* voltage reference */
		o = setup->observables + ni_reference_611x( ai_chan );
		o->name = "ai, bipolar voltage reference";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec = CR_PACK(ai_chan, range, AREF_DIFF)
			| CR_ALT_SOURCE | CR_ALT_FILTER;
		o->reference_source = ref_source_611x( REF_CALSRC_GND, cal_gain_reg_bits );
		o->target = voltage_reference;
	}

	memset(&po_tmpl,0,sizeof(po_tmpl));
	po_tmpl.insn = INSN_WRITE;
	po_tmpl.n = 1;
	po_tmpl.subdev = setup->da_subdev;

	ai_range_for_ao = 2;

	/* ao 0, zero offset */
	o = setup->observables + ni_ao0_zero_offset_611x;
	o->name = "ao 0, zero offset";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND );
	o->preobserve_insn.data = o->preobserve_data;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF )
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_DAC0_GND;
	set_target( setup, ni_ao0_zero_offset_611x, 0.0 );

	/* ao 0, gain */
	o = setup->observables + ni_ao0_reference_611x;
	o->name = "ao 0, reference voltage";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.chanspec = CR_PACK( 0, 0, AREF_GROUND );
	o->preobserve_insn.data = o->preobserve_data;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF )
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_DAC0_GND;
	set_target( setup, ni_ao0_reference_611x, 5.0 );

	/* ao 1, zero offset */
	o = setup->observables + ni_ao1_zero_offset_611x;
	o->name = "ao 1, zero offset";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND );
	o->preobserve_insn.data = o->preobserve_data;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF)
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_DAC1_GND;
	set_target( setup, ni_ao1_zero_offset_611x, 0.0 );

	/* ao 1, gain */
	o = setup->observables + ni_ao1_reference_611x;
	o->name = "ao 1, reference voltage";
	o->preobserve_insn = po_tmpl;
	o->preobserve_insn.chanspec = CR_PACK( 1, 0, AREF_GROUND );
	o->preobserve_insn.data = o->preobserve_data;
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK( 0, ai_range_for_ao, AREF_DIFF )
		| CR_ALT_SOURCE | CR_ALT_FILTER;
	o->reference_source = REF_DAC1_GND;
	set_target( setup, ni_ao1_reference_611x, 5.0 );

	setup->n_observables = 4 + 2 * num_chans;
}

static int cal_ni_at_mio_16e_2(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_zero_offset_low,2);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		cal1( setup, ni_ao0_reference,6);
		cal1( setup, ni_ao1_zero_offset,8);
		cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

/*
 * Device name: DAQCard-ai-16xe-50
 * Comedi version: 0.7.60
 * ai, bipolar zero offset, low gain
 * offset 5.87(63)e-3, target 0
 * caldac[0] gain=-2.243(21)e-6 V/bit S_min=208.079 dof=254
 * caldac[2] gain=1.56378(22)e-4 V/bit S_min=1782.91 dof=254
 * caldac[8] gain=2.499(14)e-7 V/bit S_min=234.915 dof=254
 * ai, bipolar zero offset, high gain
 * offset 4.251(49)e-5, target 0
 * caldac[0] gain=-2.396(30)e-8 V/bit S_min=231.387 dof=254
 * caldac[2] gain=1.56428(28)e-6 V/bit S_min=829.096 dof=254
 * caldac[8] gain=2.61244(18)e-7 V/bit S_min=773.092 dof=254
 * ai, bipolar voltage reference, low gain
 * offset 4.99650(81), target 5
 * caldac[0] gain=-3.78250(23)e-4 V/bit S_min=12207.6 dof=254
 * caldac[1] gain=-9.878(22)e-6 V/bit S_min=346.795 dof=254
 * caldac[2] gain=1.57172(23)e-4 V/bit S_min=969.526 dof=254
 * caldac[8] gain=2.795(14)e-7 V/bit S_min=245.703 dof=254
 * ai, unipolar zero offset, low gain
 * offset 0.0133(14), target 0
 * caldac[0] gain=3.73923(29)e-4 V/bit S_min=2855.79 dof=151
 * caldac[1] gain=9.784(11)e-6 V/bit S_min=727.295 dof=254
 * caldac[2] gain=7.8670(11)e-5 V/bit S_min=903.291 dof=254
 * caldac[8] gain=2.7732(74)e-7 V/bit S_min=415.399 dof=254
 */
static int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);
	return 0;
}

static int cal_ni_at_mio_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);

	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low, ni_zero_offset_high, 2);
	postgain_cal( setup, ni_zero_offset_low, ni_zero_offset_high, 3);
	cal1( setup, ni_zero_offset_high, 8);
	cal1( setup, ni_reference_low, 0);
	cal1( setup, ni_reference_low, 1);

	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_at_mio_16e_1(calibration_setup_t *setup)
{
	return cal_ni_at_mio_16e_2( setup );
}

static int cal_ni_pci_mio_16e_1(calibration_setup_t *setup)
{
	//cal_ni_at_mio_16e_2();

	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_zero_offset_low,2);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		//cal1( setup, ni_ao0_zero_offset,4); /* linearity? */
		cal1( setup, ni_ao0_reference,6);
		cal1( setup, ni_ao1_zero_offset,8);
		//cal1( setup, ni_ao1_zero_offset,7); /* linearity? */
		cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

static int cal_ni_pci_6032e(calibration_setup_t *setup)
{
	postgain_cal(setup, ni_zero_offset_low, ni_zero_offset_high, 2);
	postgain_cal(setup, ni_zero_offset_low, ni_zero_offset_high, 3);

	cal1( setup, ni_zero_offset_high,8);

	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);

	return 0;
}

static int cal_ni_pci_6035e(calibration_setup_t *setup)
{
	/* this is for the ad8804_debug caldac */

	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,4);

	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,8);

	cal1( setup, ni_reference_low,2);

	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		//cal1( setup, ni_ao0_zero_offset,10); /* linearity? */
		cal1( setup, ni_ao0_reference,11);
		cal1( setup, ni_ao1_zero_offset,9);
		//cal1( setup, ni_ao1_zero_offset,1); /* linearity? */
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_pci_6071e(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1_fine( setup, ni_reference_low,3);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		//cal1( setup, ni_ao0_zero_offset,4); /* linearity? */
		/* caldac 6 should most likely be AO0 reference, but it
		 * isn't. */
		/*cal1( setup, ni_ao0_reference,6);*/
		cal1( setup, ni_ao1_zero_offset,8);
		//cal1( setup, ni_ao1_zero_offset,7); /* linearity? */
		cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

static int cal_ni_pxi_6071e(calibration_setup_t *setup)
{
	// 6071e (old)
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	if(setup->do_output){
		// unknown
	}
	return 0;
}

static int cal_ni_at_mio_16e_10(calibration_setup_t *setup)
{
	// 16e-10 (old)
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,10);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_zero_offset_low,2);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,5); // guess
		cal1( setup, ni_ao0_reference,6); // guess
		cal1( setup, ni_ao1_zero_offset,8); // guess
		cal1( setup, ni_ao1_reference,9); // guess
	}
	return 0;
}

static int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);

	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_pci_6023e(calibration_setup_t *setup)
{
	/* for comedi-0.7.65 */

	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,4);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,8); /* possibly wrong */
	cal1( setup, ni_reference_low,2);

	return 0;
}

static int cal_ni_pci_6024e(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,4);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,2);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		//cal1( setup, ni_ao0_zero_offset,10); // nonlinearity?
		cal1( setup, ni_ao0_reference,11);
		cal1( setup, ni_ao1_zero_offset,9);
		//cal1( setup, ni_ao1_zero_offset,1); // nonlinearity?
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_pci_6025e(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,4);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,2);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		//cal1( setup, ni_ao0_zero_offset,10); /* nonlinearity */
		cal1( setup, ni_ao0_reference,11);
		cal1( setup, ni_ao1_zero_offset,9);
		//cal1( setup, ni_ao1_zero_offset,1); /* nonlinearity */
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_pci_6052e(calibration_setup_t *setup)
{
	/*
	 * This board has noisy caldacs
	 *
	 * The NI documentation says:
	 *   0, 8   AI pregain  (coarse, fine)		3, 11
	 *   4, 12  AI postgain				15,7
	 *   2, 10  AI reference			1, 9
	 *   14, 7  AI unipolar offset			5, 13
	 *
	 *   0      AO0 linearity
	 *   8, 4   AO0 reference		23, 19	7, 3
	 *   12     AO0 offset			27	11
	 *   2      AO1 linearity
	 *   10, 6  AO1 reference		25, 21	9, 5
	 *   14     AO1 offset			29, 17	13, 1
	 *
	 *   0	3	x	0011
	 *
	 *   2	1	x	0001
	 *
	 *   4	7	15 3	0111 0011
	 *
	 *   6		17 5	     0101
	 *   7	x
	 *   8	11	19 7	1011 0111
	 *
	 *   10	9	21 9	1001 1001
	 *
	 *   12	x	23 11	     1011
	 *
	 *   14 5	13 1	0101 0001
	 *
	 */

	cal_postgain_binary( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,3);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,1);
	cal_binary( setup, ni_reference_low,4);
	cal1_fine( setup, ni_reference_low,4);
	cal1( setup, ni_reference_low,5);
	cal1( setup, ni_unip_zero_offset_low,6);
	cal1_fine( setup, ni_unip_zero_offset_low,6);
	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset,12+11);
		cal1_fine( setup, ni_ao0_zero_offset,12+11);
		cal1( setup, ni_ao0_reference,12+7);
		cal1_fine( setup, ni_ao0_reference,12+7);
		cal1( setup, ni_ao0_reference,12+3);
		cal1( setup, ni_ao1_zero_offset,12+1);
		cal1( setup, ni_ao1_reference,12+9);
		cal1_fine( setup, ni_ao1_reference,12+9);
		cal1( setup, ni_ao1_reference,12+5);
	}
	return 0;
}

static int cal_ni_pci_mio_16e_4(calibration_setup_t *setup)
{
	/* this is for the ad8804_debug caldac */

	cal_postgain_binary( setup, ni_zero_offset_low,ni_zero_offset_high,4);
	//cal_postgain_fine( setup, ni_zero_offset_low,ni_zero_offset_high,4);
	cal1( setup, ni_zero_offset_high,8);
	cal_binary( setup, ni_reference_low,2);
	cal1_fine( setup, ni_reference_low,2);

	cal1( setup, ni_unip_zero_offset_low,7);
	cal1_fine( setup, ni_unip_zero_offset_low,7);

	if(setup->do_output){
		cal_binary( setup, ni_ao0_zero_offset,6);
		cal1_fine( setup, ni_ao0_zero_offset,6);
		//cal1( setup, ni_ao0_nonlinearity,10);
		cal_binary( setup, ni_ao0_reference,11);
		cal1_fine( setup, ni_ao0_reference,11);
		cal_binary( setup, ni_ao1_zero_offset,9);
		cal1_fine( setup, ni_ao1_zero_offset,9);
		//cal1( setup, ni_ao1_nonlinearity,1);
		cal_binary( setup, ni_ao1_reference,5);
		cal1_fine( setup, ni_ao1_reference,5);
	}
	return 0;
}

static int cal_ni_daqcard_ai_16e_4(calibration_setup_t *setup)
{
	cal_postgain_binary(setup, ni_zero_offset_low, ni_zero_offset_high, 1);
	//cal_postgain_fine(setup, ni_zero_offset_low, ni_zero_offset_high, 1);

	cal_binary( setup, ni_zero_offset_high,0);
	cal1_fine( setup, ni_zero_offset_high,0);

	cal_binary( setup, ni_reference_low,3);
	cal1_fine( setup, ni_reference_low,3);

	cal1( setup, ni_unip_zero_offset_low,2);

	return 0;
}

static int cal_ni_pci_611x( calibration_setup_t *setup )
{
	int i;
	int num_chans;

	num_chans = comedi_get_n_channels( setup->dev, setup->ad_subdev );

	for( i = 0; i < num_chans; i++ ){
		cal1( setup, ni_zero_offset_611x( i ), ( 2 * i + 2 ) );
		cal1( setup, ni_reference_611x( i ), ( 2 * i + 1 ) );
	}

	if(setup->do_output){
		cal1( setup, ni_ao0_zero_offset_611x, 14 );
		cal1( setup, ni_ao0_reference_611x, 13 );
		cal1( setup, ni_ao1_zero_offset_611x, 16 );
		cal1( setup, ni_ao1_reference_611x, 15 );
	}

	return 0;
}

static int cal_ni_daqcard_6062e( calibration_setup_t *setup )
{
	saved_calibration_t saved_cals[ 3 ], *current_cal;
	static const int num_calibrations = sizeof( saved_cals ) / sizeof( saved_cals[0] );
	int i, retval;
	enum caldacs
	{
		ADC_PREGAIN_OFFSET = 0,
		DAC1_OFFSET = 1,
		ADC_GAIN = 2,
		DAC0_GAIN = 3,
		ADC_POSTGAIN_OFFSET = 4,
		DAC1_GAIN = 5,
		DAC0_OFFSET = 6,
	};

	comedi_set_global_oor_behavior( COMEDI_OOR_NUMBER );

	current_cal = saved_cals;

	memset( saved_cals, 0, sizeof( saved_cals ) );

	cal_postgain_binary( setup, ni_zero_offset_low, ni_zero_offset_high, ADC_POSTGAIN_OFFSET );
	cal_binary( setup, ni_zero_offset_high, ADC_PREGAIN_OFFSET );
	cal_binary( setup, ni_reference_low, ADC_GAIN );

	current_cal->subdevice = setup->ad_subdev;
	sc_push_caldac( current_cal, setup->caldacs[ ADC_PREGAIN_OFFSET ] );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_GAIN ] );
	sc_push_caldac( current_cal, setup->caldacs[ ADC_POSTGAIN_OFFSET ] );
	sc_push_channel( current_cal, SC_ALL_CHANNELS );
	sc_push_range( current_cal, SC_ALL_RANGES );
	sc_push_aref( current_cal, SC_ALL_AREFS );
	current_cal++;

	if(setup->do_output){
		cal_binary( setup, ni_ao0_zero_offset, DAC0_OFFSET );
		cal_binary( setup, ni_ao0_reference, DAC0_GAIN );

		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_OFFSET ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC0_GAIN ] );
		sc_push_channel( current_cal, 0 );
		sc_push_range( current_cal, SC_ALL_RANGES );
		sc_push_aref( current_cal, SC_ALL_AREFS );
		current_cal++;

		cal_binary( setup, ni_ao1_zero_offset, DAC1_OFFSET );
		cal_binary( setup, ni_ao1_reference, DAC1_GAIN );

		current_cal->subdevice = setup->da_subdev;
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_OFFSET ] );
		sc_push_caldac( current_cal, setup->caldacs[ DAC1_GAIN ] );
		sc_push_channel( current_cal, 1 );
		sc_push_range( current_cal, SC_ALL_RANGES );
		sc_push_aref( current_cal, SC_ALL_AREFS );
		current_cal++;
	}

	retval = write_calibration_file( setup, saved_cals, num_calibrations );
	for( i = 0; i < num_calibrations; i++ )
		clear_saved_calibration( &saved_cals[ i ] );

	return retval;
}

static double ni_get_reference( calibration_setup_t *setup, int lsb_loc,int msb_loc)
{
	int lsb,msb;
	int16_t uv;
	double ref;

	lsb=read_eeprom( setup, lsb_loc);
	msb=read_eeprom( setup, msb_loc);
	printf("lsb=%d msb=%d\n", lsb, msb);

	uv = ( lsb & 0xff ) | ( ( msb << 8 ) & 0xff00 );
	ref=5.000+1.0e-6*uv;
	printf("ref=%g\n",ref);

	return ref;
}

#if 0
static void cal_ni_results(void)
{
	comedi_range *range;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	//int have_ao;
	char s[32];

	bipolar_lowgain = get_bipolar_lowgain(dev,setup->ad_subdev);
	bipolar_highgain = get_bipolar_highgain(dev,setup->ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain(dev,setup->ad_subdev);

	/* 0 offset, low gain */
	range = comedi_get_range(dev,setup->ad_subdev,0,bipolar_lowgain);
	read_chan2(s,0,bipolar_lowgain);
	DPRINT(0,"bipolar zero offset, low gain [%g,%g]: %s\n",
		range->min,range->max,s);

	/* 0 offset, high gain */
	range = comedi_get_range(dev,setup->ad_subdev,0,bipolar_highgain);
	read_chan2(s,0,bipolar_highgain);
	DPRINT(0,"bipolar zero offset, high gain [%g,%g]: %s\n",
		range->min,range->max,s);

	/* unip/bip offset */
	range = comedi_get_range(dev,setup->ad_subdev,0,unipolar_lowgain);
	read_chan2(s,0,unipolar_lowgain);
	DPRINT(0,"unipolar zero offset, low gain [%g,%g]: %s\n",
		range->min,range->max,s);

}

static void ni_mio_ai_postgain_cal(void)
{
	linear_fit_t l;
	double offset_r0;
	double offset_r7;
	double gain;
	double a;

	check_gain_chan_x(&l,CR_PACK(0,0,AREF_OTHER),1);
	offset_r0=linear_fit_func_y(&l,caldacs[1].current);
	printf("offset r0 %g\n",offset_r0);

	check_gain_chan_x(&l,CR_PACK(0,7,AREF_OTHER),1);
	offset_r7=linear_fit_func_y(&l,caldacs[1].current);
	printf("offset r7 %g\n",offset_r7);

	gain=l.slope;

	a=(offset_r0-offset_r7)/(200.0-1.0);
	a=caldacs[1].current-a/gain;

	printf("%g\n",a);

	caldacs[1].current=rint(a);
	update_caldac(1);
}

static void ni_mio_ai_postgain_cal_2(int chan,int dac,int range_lo,int range_hi,double gain)
{
	double offset_lo,offset_hi;
	linear_fit_t l;
	double slope;
	double a;

	check_gain_chan_x(&l,CR_PACK(chan,range_lo,AREF_OTHER),dac);
	offset_lo=linear_fit_func_y(&l,caldacs[dac].current);
	printf("offset lo %g\n",offset_lo);

	check_gain_chan_x(&l,CR_PACK(chan,range_hi,AREF_OTHER),dac);
	offset_hi=linear_fit_func_y(&l,caldacs[dac].current);
	printf("offset hi %g\n",offset_hi);

	slope=l.slope;

	a=(offset_lo-offset_hi)/(gain-1.0);
	a=caldacs[dac].current-a/slope;

	printf("%g\n",a);

	caldacs[dac].current=rint(a);
	update_caldac(dac);
}
#endif

