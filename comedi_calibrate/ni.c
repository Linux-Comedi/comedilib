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


char ni_id[] = "$Id$";

struct board_struct{
	char *name;
	int status;
	int (*cal)( calibration_setup_t *setup);
};

int ni_setup_board( calibration_setup_t *setup , const char *device_name );
void ni_setup_observables( calibration_setup_t *setup );

int cal_ni_at_mio_16e_2(calibration_setup_t *setup);
int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup);
int cal_ni_at_mio_16e_1(calibration_setup_t *setup);
int cal_ni_pci_mio_16e_1(calibration_setup_t *setup);
int cal_ni_pci_6025e(calibration_setup_t *setup);
int cal_ni_pci_6035e(calibration_setup_t *setup);
int cal_ni_pci_6071e(calibration_setup_t *setup);
int cal_ni_pxi_6071e(calibration_setup_t *setup);
int cal_ni_at_mio_16e_10(calibration_setup_t *setup);
int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup);
int cal_ni_pci_6023e(calibration_setup_t *setup);
int cal_ni_pci_6024e(calibration_setup_t *setup);
int cal_ni_at_mio_16xe_50(calibration_setup_t *setup);
int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup);
int cal_ni_pci_6052e(calibration_setup_t *setup);

static struct board_struct boards[]={
	{ "at-mio-16e-2",	STATUS_DONE,	cal_ni_at_mio_16e_2 },
	{ "DAQCard-ai-16xe-50",	STATUS_DONE,	cal_ni_daqcard_ai_16xe_50 },
	{ "at-mio-16xe-50",	STATUS_SOME,	cal_ni_at_mio_16xe_50 },
	{ "at-mio-16e-1",	STATUS_SOME,	cal_ni_at_mio_16e_1 },
	{ "pci-mio-16e-1",	STATUS_DONE,	cal_ni_pci_mio_16e_1 },
	{ "pci-6025e",		STATUS_SOME,	cal_ni_pci_6025e },
	{ "pci-6035e",		STATUS_GUESS,	cal_ni_pci_6035e },
	{ "pci-6071e",		STATUS_SOME,	cal_ni_pci_6071e },
	{ "pxi-6071e",		STATUS_GUESS,	cal_ni_pxi_6071e },
	{ "at-mio-16e-10",	STATUS_GUESS,	cal_ni_at_mio_16e_10 },
	{ "pci-mio-16xe-50",	STATUS_SOME,	cal_ni_pci_mio_16xe_50 },
	{ "pci-6023e",		STATUS_SOME,	cal_ni_pci_6023e },
	{ "pci-mio-16xe-10",	STATUS_DONE,	cal_ni_pci_mio_16xe_10 },
	{ "pci-6052e",		STATUS_SOME,	cal_ni_pci_6052e },
	{ "pci-6024e",		STATUS_SOME,	cal_ni_pci_6024e },
#if 0
//	{ "at-mio-16de-10",	cal_ni_unknown },
	{ "at-mio-64e-3",	cal_ni_16e_1 },
//	{ "at-mio-16xe-50",	cal_ni_unknown },
//	{ "at-mio-16xe-10",	cal_ni_unknown },
//	{ "at-ai-16xe-10",	cal_ni_unknown },
//	{ "pxi-6030e",		cal_ni_unknown },
//	{ "pci-mio-16e-4",	cal_ni_unknown },
//	{ "pxi-6040e",		cal_ni_unknown },
//	{ "pci-6031e",		cal_ni_unknown },
//	{ "pci-6032e",		cal_ni_unknown },
//	{ "pci-6033e",		cal_ni_unknown },
	{ "pxi-6025e",		cal_ni_6023e }, // guess
	{ "pci-6034e",		cal_ni_6023e }, // guess
//	{ "pci-6110e",		cal_ni_unknown },
//	{ "pci-6111e",		cal_ni_unknown },
//	{ "pci-6711",		cal_ni_unknown },
//	{ "pci-6713",		cal_ni_unknown },
//	{ "pxi-6070e",		cal_ni_unknown },
//	{ "pxi-6052e",		cal_ni_unknown },
//	{ "DAQCard-ai-16e-4",	cal_ni_unknown },
//	{ "DAQCard-6062e",	cal_ni_unknown },
//	{ "DAQCard-6024e",	cal_ni_unknown },
#endif
};
#define n_boards (sizeof(boards)/sizeof(boards[0]))

enum {
	ni_zero_offset_low = 0,
	ni_zero_offset_high,
	ni_reference_low,
	ni_unip_offset_low,
	ni_ao0_zero_offset,
	ni_ao0_reference,
	ni_ao1_zero_offset,
	ni_ao1_reference,
};

int ni_setup( calibration_setup_t *setup , const char *device_name )
{
	ni_setup_board( setup, device_name );
	ni_setup_observables( setup );
	setup_caldacs( setup, caldac_subdev );

	return 0;
}

int ni_setup_board( calibration_setup_t *setup, const char *device_name )
{
	int i;

	for(i = 0; i < n_boards; i++ ){
		if(!strcmp( device_name, boards[i].name )){
			setup->status = boards[i].status;
			setup->do_cal = boards[i].cal;
			break;
		}
	}
	return 0;
}

void ni_setup_observables( calibration_setup_t *setup )
{
	comedi_insn tmpl;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	double voltage_reference;
	observable *o;

	bipolar_lowgain = get_bipolar_lowgain( setup->dev, ad_subdev);
	bipolar_highgain = get_bipolar_highgain( setup->dev, ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain( setup->dev, ad_subdev);

	voltage_reference = 5.000;

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = ad_subdev;

	/* 0 offset, low gain */
	o = setup->observables + ni_zero_offset_low;
	o->name = "ai, bipolar zero offset, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0,bipolar_lowgain,AREF_OTHER);
	o->target = 0;

	/* 0 offset, high gain */
	o = setup->observables + ni_zero_offset_high;
	o->name = "ai, bipolar zero offset, high gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0,bipolar_highgain,AREF_OTHER);
	o->target = 0;

	/* voltage reference */
	o = setup->observables + ni_reference_low;
	o->name = "ai, bipolar voltage reference, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(5,bipolar_lowgain,AREF_OTHER);
	o->target = voltage_reference;

	setup->n_observables = ni_reference_low + 1;

	if(unipolar_lowgain>=0){
		/* unip/bip offset */
		o = setup->observables + ni_unip_offset_low;
		o->name = "ai, unipolar zero offset, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(0,unipolar_lowgain,AREF_OTHER);
		o->target = 0;

#if 0
		/* unip gain */
		o = observables + ni_unip_reference_low;
		o->name = "ai, unipolar voltage reference, low gain";
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(5,unipolar_lowgain,AREF_OTHER);
		o->target = voltage_reference;
		i++;
#endif
		setup->n_observables = ni_unip_offset_low + 1;
	}

	if(da_subdev>=0){
		comedi_insn po_tmpl;

		memset(&po_tmpl,0,sizeof(po_tmpl));
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = da_subdev;

		/* ao 0, zero offset */
		o = setup->observables + ni_ao0_zero_offset;
		o->name = "ao 0, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(2,bipolar_lowgain,AREF_OTHER);
		set_target( setup, ni_ao0_zero_offset,0.0);

		/* ao 0, gain */
		o = setup->observables + ni_ao0_reference;
		o->name = "ao 0, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(6,bipolar_lowgain,AREF_OTHER);
		set_target( setup, ni_ao0_reference,5.0);
		o->target -= voltage_reference;

		/* ao 1, zero offset */
		o = setup->observables + ni_ao1_zero_offset;
		o->name = "ao 1, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(3,bipolar_lowgain,AREF_OTHER);
		set_target( setup, ni_ao1_zero_offset,0.0);

		/* ao 1, gain */
		o = setup->observables + ni_ao1_reference;
		o->name = "ao 1, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(7,bipolar_lowgain,AREF_OTHER);
		set_target( setup, ni_ao1_reference,5.0);
		o->target -= voltage_reference;

		setup->n_observables = ni_ao1_reference + 1;
	}
}

int cal_ni_at_mio_16e_2(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_offset_low,2);
	if(do_output){
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
int cal_ni_daqcard_ai_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);
	return 0;
}

int cal_ni_at_mio_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);

	if(do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

int cal_ni_pci_mio_16xe_10(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low, ni_zero_offset_high, 2);
	postgain_cal( setup, ni_zero_offset_low, ni_zero_offset_high, 3);
	cal1( setup, ni_zero_offset_high, 8);
	cal1( setup, ni_reference_low, 0);
	cal1( setup, ni_reference_low, 1);

	if(do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

int cal_ni_at_mio_16e_1(calibration_setup_t *setup)
{
	return cal_ni_at_mio_16e_2( setup );
}

int cal_ni_pci_mio_16e_1(calibration_setup_t *setup)
{
	//cal_ni_at_mio_16e_2();

	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_offset_low,2);
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		//cal1( setup, ni_ao0_zero_offset,4); /* linearity? */
		cal1( setup, ni_ao0_reference,6);
		cal1( setup, ni_ao1_zero_offset,8);
		//cal1( setup, ni_ao1_zero_offset,7); /* linearity? */
		cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

int cal_ni_pci_6035e(calibration_setup_t *setup)
{
	// 6035e (old)
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	if(do_output){
		// unknown
	}
	return 0;
}

int cal_ni_pci_6071e(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1_fine( setup, ni_reference_low,3);
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		//cal1( setup, ni_ao0_zero_offset,4); /* linearity? */
		cal1( setup, ni_ao0_reference,6); /* guess.  Doesn't show up correctly in dump */
		cal1( setup, ni_ao1_zero_offset,8);
		//cal1( setup, ni_ao1_zero_offset,7); /* linearity? */
		cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

int cal_ni_pxi_6071e(calibration_setup_t *setup)
{
	// 6071e (old)
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	if(do_output){
		// unknown
	}
	return 0;
}

int cal_ni_at_mio_16e_10(calibration_setup_t *setup)
{
	// 16e-10 (old)
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	cal1( setup, ni_zero_offset_high,10);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	cal1( setup, ni_unip_offset_low,2);
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,5); // guess
		cal1( setup, ni_ao0_reference,6); // guess
		cal1( setup, ni_ao1_zero_offset,8); // guess
		cal1( setup, ni_ao1_reference,9); // guess
	}
	return 0;
}

int cal_ni_pci_mio_16xe_50(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	cal1( setup, ni_zero_offset_high,8);
	cal1( setup, ni_reference_low,0);
	cal1_fine( setup, ni_reference_low,0);
	cal1( setup, ni_reference_low,1);

	if(do_output){
		cal1( setup, ni_ao0_zero_offset,6);
		cal1( setup, ni_ao0_reference,4);
		cal1( setup, ni_ao1_zero_offset,7);
		cal1( setup, ni_ao1_reference,5);
	}
	return 0;
}

int cal_ni_pci_6023e(calibration_setup_t *setup)
{
	/* There seems to be a bug in the driver that doesn't allow
	 * access to caldac 10, and possibly others. */
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	//cal1( setup, ni_zero_offset_high,10);
	//cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	return 0;
}

int cal_ni_pci_6024e(calibration_setup_t *setup)
{
	/* There seems to be a bug in the driver that doesn't allow
	 * access to caldac 10, and possibly others. */
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,1);
	//cal1( setup, ni_zero_offset_high,10);
	//cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_reference_low,3);
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,5);
		//cal1( setup, ni_ao0_zero_offset,4); // nonlinearity?
		//cal1( setup, ni_ao0_reference,6);
		cal1( setup, ni_ao1_zero_offset,8);
		//cal1( setup, ni_ao1_zero_offset,7); // nonlinearity?
		//cal1( setup, ni_ao1_reference,9);
	}
	return 0;
}

int cal_ni_pci_6025e(calibration_setup_t *setup)
{
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,4); // was 1
	//cal1( setup, ni_zero_offset_high,XXX); // was 10
	cal1( setup, ni_zero_offset_high,8); // was 0
	cal1( setup, ni_reference_low,2); // was 3
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,6); // was 5
		//cal1( setup, ni_ao0_zero_offset,10); // nonlinearity was 4
		//cal1( setup, ni_ao0_reference,XXX); // was 6
		cal1( setup, ni_ao1_zero_offset,9); // was 8
		//cal1( setup, ni_ao1_zero_offset,1); // nonlinearity was 7
		cal1( setup, ni_ao1_reference,5); // was 9
	}
	return 0;
}

int cal_ni_pci_6052e(calibration_setup_t *setup)
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

	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,2);
	postgain_cal( setup, ni_zero_offset_low,ni_zero_offset_high,3);
	cal1( setup, ni_zero_offset_high,0);
	cal1( setup, ni_zero_offset_high,1);
	cal1( setup, ni_reference_low,4);
	cal1_fine( setup, ni_reference_low,4);
	cal1( setup, ni_reference_low,5);
	cal1( setup, ni_unip_offset_low,6);
	cal1_fine( setup, ni_unip_offset_low,6);
	//cal1( setup, ni_unip_offset_low,7);
	if(do_output){
		cal1( setup, ni_ao0_zero_offset,12+11);
		cal1( setup, ni_ao0_reference,12+7);
		cal1( setup, ni_ao0_reference,12+3);
		cal1( setup, ni_ao1_zero_offset,12+1);
		cal1( setup, ni_ao1_reference,12+9);
		cal1( setup, ni_ao1_reference,12+5);
	}
	return 0;
}

double ni_get_reference( calibration_setup_t *setup, int lsb_loc,int msb_loc)
{
	int lsb,msb;
	int uv;
	double ref;

	lsb=read_eeprom( setup, lsb_loc);
	msb=read_eeprom( setup, msb_loc);
	printf("lsb=%d msb=%d\n",read_eeprom( setup, 425),read_eeprom( setup, 426));

	uv=lsb | (msb<<8);
	if(uv>=0x8000)uv-=0x10000;
	ref=5.000+1.0e-6*uv;
	printf("ref=%g\n",ref);

	return ref;
}

#if 0
void cal_ni_results(void)
{
	comedi_range *range;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	//int have_ao;
	char s[32];

	bipolar_lowgain = get_bipolar_lowgain(dev,ad_subdev);
	bipolar_highgain = get_bipolar_highgain(dev,ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain(dev,ad_subdev);

	/* 0 offset, low gain */
	range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
	read_chan2(s,0,bipolar_lowgain);
	DPRINT(0,"bipolar zero offset, low gain [%g,%g]: %s\n",
		range->min,range->max,s);

	/* 0 offset, high gain */
	range = comedi_get_range(dev,ad_subdev,0,bipolar_highgain);
	read_chan2(s,0,bipolar_highgain);
	DPRINT(0,"bipolar zero offset, high gain [%g,%g]: %s\n",
		range->min,range->max,s);

	/* unip/bip offset */
	range = comedi_get_range(dev,ad_subdev,0,unipolar_lowgain);
	read_chan2(s,0,unipolar_lowgain);
	DPRINT(0,"unipolar zero offset, low gain [%g,%g]: %s\n",
		range->min,range->max,s);

}

void ni_mio_ai_postgain_cal(void)
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

void ni_mio_ai_postgain_cal_2(int chan,int dac,int range_lo,int range_hi,double gain)
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

