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

#define DPRINT(level,fmt,args...) do{if(verbose>=level)printf(fmt, ## args);}while(0)

#define N_CALDACS 32
#define N_OBSERVABLES 32

typedef struct{
	int subdev;
	int chan;

	int maxdata;
	int current;

	int type;
	double gain;
}caldac;

typedef struct{
	char *name;

	comedi_insn preobserve_insn;
	lsampl_t preobserve_data;

	comedi_insn observe_insn;

	//comedi_range *range;
	//int maxdata;

	double target;
}observable;

static caldac caldacs[N_CALDACS];
static observable observables[N_OBSERVABLES];

static int n_caldacs;
static int n_observables;

static comedi_t *dev;

static int ad_subdev;
static int da_subdev;
static int eeprom_subdev;
static int caldac_subdev;

double read_chan(int adc,int range);
int read_chan2(char *s,int adc,int range);
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value);
void check_gain(int ad_chan,int range);
double check_gain_chan(int ad_chan,int range,int cdac);

int verbose = 0;

enum {
	STATUS_UNKNOWN = 0,
	STATUS_SOME,
	STATUS_DONE
};
int device_status = STATUS_UNKNOWN;

/* tmep */
void do_cal(void);

void observe(void);
void preobserve(int obs);
void observable_dependence(int obs);
void measure_observable(int obs);
void ni_setup(void);
void set_target(int obs,double target);

void cal_ni_results(void);

void update_caldac(int i);
void reset_caldacs(void);
void setup_caldacs(void);
void cal_ni_mio_E(void);
void ni_mio_ai_postgain_cal(void);
void ni_mio_ai_postgain_cal_2(int chan,int dac,int range_lo,int range_hi,double gain);
void channel_dependence(int adc,int range);
void caldac_dependence(int caldac);
void dump_curve(int adc,int caldac);
void chan_cal(int adc,int caldac,int range,double target);
int read_eeprom(int addr);

int get_bipolar_lowgain(comedi_t *dev,int subdev);
int get_bipolar_highgain(comedi_t *dev,int subdev);
int get_unipolar_lowgain(comedi_t *dev,int subdev);

int sci_sprint(char *s,double x,double y);
int sci_sprint_alt(char *s,double x,double y);


typedef struct {
	int n;

	double *y_data;
	double *yerr_data;
	double *x_data;

	double x0;
	double dx;
	double yerr;

	/* stats */
	double s1,sx,sy,sxy,sxx;

	double min,max;

	/* results */
	double ave_x;
	double ave_y;
	double slope;
	double err_slope;
	double err_ave_y;
	double S_min;
	double dof;

}linear_fit_t;
int linear_fit_monotonic(linear_fit_t *l);
double linear_fit_func_y(linear_fit_t *l,double x);
double check_gain_chan_x(linear_fit_t *l,unsigned int ad_chanspec,int cdac);

typedef struct{
	comedi_t *dev;

	int maxdata;
	int order;
	int aref;
	int range;
	int subd;
	int chan;

	comedi_range *rng;

	int n;
	double average;
	double stddev;
	double error;
}new_sv_t;

int new_sv_measure(new_sv_t *sv);
int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,int chan,int range,int aref);

struct board_struct{
	char *name;
	void (*setup)(void);
};

#if 0
void cal_ni_16e_1(void);
void cal_ni_16e_10(void);
void cal_ni_16xe_50(void);
void cal_ni_16xe_10(void);
void cal_ni_6023e(void);
void cal_ni_6071e(void);
void cal_ni_daqcard_ai_16xe_50(void);
void cal_ni_unknown(void);

struct board_struct boards[]={
	{ "at-mio-16e-1",	cal_ni_16e_1 },
	{ "at-mio-16e-2",	cal_ni_16e_1 },
	{ "at-mio-16e-10",	cal_ni_16e_10 },
//	{ "at-mio-16de-10",	cal_ni_unknown },
	{ "at-mio-64e-3",	cal_ni_16e_1 },
//	{ "at-mio-16xe-50",	cal_ni_unknown },
//	{ "at-mio-16xe-10",	cal_ni_unknown },
//	{ "at-ai-16xe-10",	cal_ni_unknown },
	{ "pci-mio-16xe-50",	cal_ni_16xe_50 },
	{ "pci-mio-16xe-10",	cal_ni_16xe_10 },
//	{ "pxi-6030e",		cal_ni_unknown },
	{ "pci-mio-16e-1",	cal_ni_16e_1 },
//	{ "pci-mio-16e-4",	cal_ni_unknown },
//	{ "pxi-6040e",		cal_ni_unknown },
//	{ "pci-6031e",		cal_ni_unknown },
//	{ "pci-6032e",		cal_ni_unknown },
//	{ "pci-6033e",		cal_ni_unknown },
//	{ "pci-6071e",		cal_ni_unknown },
	{ "pci-6023e",		cal_ni_6023e },
	{ "pci-6024e",		cal_ni_6023e }, // guess
	{ "pci-6025e",		cal_ni_6023e }, // guess
	{ "pxi-6025e",		cal_ni_6023e }, // guess
	{ "pci-6034e",		cal_ni_6023e }, // guess
	{ "pci-6035e",		cal_ni_6023e },
//	{ "pci-6052e",		cal_ni_unknown },
//	{ "pci-6110e",		cal_ni_unknown },
//	{ "pci-6111e",		cal_ni_unknown },
//	{ "pci-6711",		cal_ni_unknown },
//	{ "pci-6713",		cal_ni_unknown },
	{ "pxi-6071e",		cal_ni_6071e },
//	{ "pxi-6070e",		cal_ni_unknown },
//	{ "pxi-6052e",		cal_ni_unknown },
	{ "DAQCard-ai-16xe-50",	cal_ni_daqcard_ai_16xe_50 },
//	{ "DAQCard-ai-16e-4",	cal_ni_unknown },
//	{ "DAQCard-6062e",	cal_ni_unknown },
//	{ "DAQCard-6024e",	cal_ni_unknown },
};
#define n_boards (sizeof(boards)/sizeof(boards[0]))
#endif

struct board_struct drivers[] = {
	{ "ni_pcimio",		ni_setup },
	{ "ni_atmio",		ni_setup },
	{ "ni_mio_cs",		ni_setup },
};
#define n_drivers (sizeof(drivers)/sizeof(drivers[0]))

int do_dump = 0;
int do_reset = 1;
int do_calibrate = 1;
int do_results = 1;
int do_output = 1;

struct option options[] = {
	{ "verbose", 0, 0, 'v' },
	{ "quiet", 0, 0, 'q' },
	{ "file", 1, 0, 'f' },
	{ "driver-name", 1, 0, 0x1000 },
	{ "device-name", 1, 0, 0x1001 },
	{ "reset", 0, &do_reset, 1 },
	{ "no-reset", 0, &do_reset, 0 },
	{ "calibrate", 0, &do_calibrate, 1 },
	{ "no-calibrate", 0, &do_calibrate, 0 },
	{ "dump", 0, &do_dump, 1 },
	{ "no-dump", 0, &do_dump, 0 },
	{ "results", 0, &do_results, 1 },
	{ "no-results", 0, &do_results, 0 },
	{ 0 },
};

int main(int argc, char *argv[])
{
	char *fn = NULL;
	int c;
	char *drivername = NULL;
	char *devicename = NULL;
	int i;
	struct board_struct *this_board;
	int index;

	fn = "/dev/comedi0";
	while (1) {
		c = getopt_long(argc, argv, "f:vq", options, &index);
		if (c == -1)break;
		switch (c) {
		case 'f':
			fn = optarg;
			break;
		case 'v':
			verbose++;
			break;
		case 'q':
			verbose--;
			break;
		case 0x1000:
			drivername = optarg;
			break;
		case 0x1001:
			devicename = optarg;
			break;
		default:
			printf("bad option\n");
			exit(1);
		}
	}

	dev = comedi_open(fn);
	if (dev == NULL ) {
		perror(fn);
		exit(0);
	}

	if(!drivername)
		drivername=comedi_get_driver_name(dev);
	if(!devicename)
		devicename=comedi_get_board_name(dev);

	ad_subdev=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_AI,0);
	da_subdev=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_AO,0);
	caldac_subdev=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_CALIB,0);
	eeprom_subdev=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_MEMORY,0);

	for(i=0;i<n_drivers;i++){
		if(!strcmp(drivers[i].name,drivername)){
			this_board = drivers+i;
			goto ok;
		}
	}
	printf("Driver %s unknown\n",drivername);
	return 1;

ok:
	this_board->setup();

	if(device_status<STATUS_DONE){
		printf("Warning: device not fully calibrated due to insufficient information\n");
		printf("Please send this output to <ds@schleef.org>\n");
		if(verbose<0)verbose=0;
		if(device_status==STATUS_UNKNOWN){
			do_reset=1;
			do_dump=1;
			do_calibrate=0;
			do_results=0;
		}
		if(device_status==STATUS_SOME){
			do_reset=1;
			do_dump=1;
			do_calibrate=1;
			do_results=1;
		}
	}
	if(verbose>=0){
		printf("$Id$\n");
		printf("Driver name: %s\n",drivername);
		printf("Device name: %s\n",devicename);
		printf("Comedi version: %d.%d.%d\n",
			(comedi_get_version_code(dev)>>16)&0xff,
			(comedi_get_version_code(dev)>>8)&0xff,
			(comedi_get_version_code(dev))&0xff);
	}

	if(do_reset)reset_caldacs();
	if(do_dump)observe();
	if(do_calibrate)do_cal();
	if(do_results)observe();

	return 0;
}

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

void ni_setup(void)
{
	comedi_insn tmpl;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	double voltage_reference;
	observable *o;

	bipolar_lowgain = get_bipolar_lowgain(dev,ad_subdev);
	bipolar_highgain = get_bipolar_highgain(dev,ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain(dev,ad_subdev);

	voltage_reference = 5.000;

	memset(&tmpl,0,sizeof(tmpl));
	tmpl.insn = INSN_READ;
	tmpl.n = 1;
	tmpl.subdev = ad_subdev;

	/* 0 offset, low gain */
	o = observables + ni_zero_offset_low;
	o->name = "ai, bipolar zero offset, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0,bipolar_lowgain,AREF_OTHER);
	o->target = 0;

	/* 0 offset, high gain */
	o = observables + ni_zero_offset_high;
	o->name = "ai, bipolar zero offset, high gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(0,bipolar_highgain,AREF_OTHER);
	o->target = 0;

	/* voltage reference */
	o = observables + ni_reference_low;
	o->name = "ai, bipolar voltage reference, low gain";
	o->observe_insn = tmpl;
	o->observe_insn.chanspec = CR_PACK(5,bipolar_lowgain,AREF_OTHER);
	o->target = voltage_reference;

	if(unipolar_lowgain>=0){
		/* unip/bip offset */
		o = observables + ni_unip_offset_low;
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
	}

	if(da_subdev>=0){
		comedi_insn po_tmpl;

		memset(&po_tmpl,0,sizeof(po_tmpl));
		po_tmpl.insn = INSN_WRITE;
		po_tmpl.n = 1;
		po_tmpl.subdev = da_subdev;

		/* ao 0, zero offset */
		o = observables + ni_ao0_zero_offset;
		o->name = "ao 0, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(2,bipolar_lowgain,AREF_OTHER);
		set_target(ni_ao0_zero_offset,0.0);

		/* ao 0, gain */
		o = observables + ni_ao0_reference;
		o->name = "ao 0, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(0,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(6,bipolar_lowgain,AREF_OTHER);
		set_target(ni_ao0_reference,5.0);
		o->target -= voltage_reference;

		/* ao 1, zero offset */
		o = observables + ni_ao1_zero_offset;
		o->name = "ao 1, zero offset, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(3,bipolar_lowgain,AREF_OTHER);
		set_target(ni_ao1_zero_offset,0.0);

		/* ao 1, gain */
		o = observables + ni_ao1_reference;
		o->name = "ao 1, reference voltage, low gain";
		o->preobserve_insn = po_tmpl;
		o->preobserve_insn.chanspec = CR_PACK(1,0,0);
		o->preobserve_insn.data = &o->preobserve_data;
		o->observe_insn = tmpl;
		o->observe_insn.chanspec =
			CR_PACK(7,bipolar_lowgain,AREF_OTHER);
		set_target(ni_ao1_reference,5.0);
		o->target -= voltage_reference;

	}
	n_observables = ni_ao1_reference + 1;

	setup_caldacs();


}

void set_target(int obs,double target)
{
	comedi_range *range;
	lsampl_t maxdata, data;

	range = comedi_get_range(dev,observables[obs].preobserve_insn.subdev,
		CR_CHAN(observables[obs].preobserve_insn.chanspec),
		CR_RANGE(observables[obs].preobserve_insn.chanspec));
	maxdata = comedi_get_maxdata(dev,observables[obs].preobserve_insn.subdev,
		CR_CHAN(observables[obs].preobserve_insn.chanspec));

	data = comedi_from_phys(target,range,maxdata);

	observables[obs].preobserve_data = data;
	observables[obs].target = comedi_to_phys(data,range,maxdata);
}

void observe(void)
{
	int i;

	for(i=0;i<n_observables;i++){
		preobserve(i);
		DPRINT(0,"%s\n",observables[i].name);
		measure_observable(i);
		observable_dependence(i);
	}

}

void preobserve(int obs)
{
	if(observables[obs].preobserve_insn.n!=0){
		comedi_do_insn(dev,&observables[obs].preobserve_insn);
	}
}

void measure_observable(int obs)
{
	char s[32];
	int n;
	new_sv_t sv;

	new_sv_init(&sv,dev,
		observables[obs].observe_insn.subdev,
		CR_CHAN(observables[obs].observe_insn.chanspec),
		CR_RANGE(observables[obs].observe_insn.chanspec),
		CR_AREF(observables[obs].observe_insn.chanspec));
	sv.order=7;
	n=new_sv_measure(&sv);

	sci_sprint_alt(s,sv.average,sv.error);
	printf("offset %s, target %g\n",s,observables[obs].target);
}

void observable_dependence(int obs)
{
	int i;
	linear_fit_t l;

	for(i=0;i<n_caldacs;i++){
		check_gain_chan_x(&l,
			observables[obs].observe_insn.chanspec,i);
	}

}


void postgain_cal(int obs1, int obs2, int dac)
{
	double offset1,offset2;
	linear_fit_t l;
	double slope1,slope2;
	double a;
	double gain;
	comedi_range *range1,*range2;

	DPRINT(0,"postgain calibration\n");
	preobserve(obs1);
	check_gain_chan_x(&l,observables[obs1].observe_insn.chanspec,dac);
	offset1=linear_fit_func_y(&l,caldacs[dac].current);
	DPRINT(1,"obs1: [%d] offset %g\n",obs1,offset1);
	range1 = comedi_get_range(dev,observables[obs1].observe_insn.subdev,
		CR_CHAN(observables[obs1].observe_insn.chanspec),
		CR_RANGE(observables[obs1].observe_insn.chanspec));
	slope1=l.slope;

	preobserve(obs2);
	check_gain_chan_x(&l,observables[obs2].observe_insn.chanspec,dac);
	offset2=linear_fit_func_y(&l,caldacs[dac].current);
	DPRINT(1,"obs2: [%d] offset %g\n",obs2,offset2);
	range2 = comedi_get_range(dev,observables[obs2].observe_insn.subdev,
		CR_CHAN(observables[obs2].observe_insn.chanspec),
		CR_RANGE(observables[obs2].observe_insn.chanspec));
	slope2=l.slope;

	gain = (range1->max-range1->min)/(range2->max-range2->min);
	DPRINT(3,"range1 %g range2 %g\n", range1->max-range1->min,
		range2->max-range2->min);
	DPRINT(2,"gain: %g\n",gain);

	DPRINT(2,"difference: %g\n",offset2-offset1);

	a = (offset1-offset2)/(slope1-slope2);
	a=caldacs[dac].current-a;

	DPRINT(0,"caldac[%d] set to %g\n",dac,a);

	caldacs[dac].current=rint(a);
	update_caldac(dac);
	usleep(100000);

	if(verbose>=2){
		preobserve(obs1);
		measure_observable(obs1);
		preobserve(obs2);
		measure_observable(obs2);
	}
}

void cal1(int obs, int dac)
{
	linear_fit_t l;
	double offset;
	double target;
	double gain;
	double a;

	DPRINT(0,"cal1\n");
	preobserve(obs);
	check_gain_chan_x(&l,observables[obs].observe_insn.chanspec,dac);
	offset=linear_fit_func_y(&l,caldacs[dac].current);
	gain=l.slope;
	
	target = observables[obs].target;
	a=caldacs[dac].current+(target-offset)/gain;

	caldacs[dac].current=rint(a);
	update_caldac(dac);
	usleep(100000);

	DPRINT(0,"caldac[%d] set to %g\n",dac,a);
	if(verbose>=2){
		measure_observable(obs);
	}
}

void do_cal(void)
{
#if 0
	// daqcard
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,2);
	cal1(ni_zero_offset_high,8);
	cal1(ni_reference_low,0);
#endif

	// 16e-2
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,1);
	cal1(ni_zero_offset_high,0);
	cal1(ni_reference_low,3);
	cal1(ni_unip_offset_low,2);
	if(do_output){
		cal1(ni_ao0_zero_offset,5);
		cal1(ni_ao0_reference,6);
		cal1(ni_ao1_zero_offset,8);
		cal1(ni_ao1_reference,9);
	}

#if 0
	// 16e-10 (old)
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,1);
	cal1(ni_zero_offset_high,10);
	cal1(ni_zero_offset_high,0);
	cal1(ni_reference_low,3);
	cal1(ni_unip_offset_low,2);
	if(do_output){
		cal1(ni_ao0_zero_offset,5); // guess
		cal1(ni_ao0_reference,6); // guess
		cal1(ni_ao0_zero_offset,8); // guess
		cal1(ni_ao0_reference,9); // guess
	}
#endif

#if 0
	// 16xe-50 (old) (same as daqcard?)
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,2);
	cal1(ni_zero_offset_high,8);
	cal1(ni_reference_low,0);
	if(do_output){
		// unknown
	}
#endif

#if 0
	// 6035e (old)
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,1);
	cal1(ni_zero_offset_high,0);
	cal1(ni_reference_low,3);
	if(do_output){
		// unknown
	}
#endif

#if 0
	// 6071e (old)
	postgain_cal(ni_zero_offset_low,ni_zero_offset_high,1);
	cal1(ni_zero_offset_high,0);
	cal1(ni_reference_low,3);
	if(do_output){
		// unknown
	}
#endif
}


double ni_get_reference(int lsb_loc,int msb_loc)
{
	int lsb,msb;
	int uv;
	double ref;

	lsb=read_eeprom(lsb_loc);
	msb=read_eeprom(msb_loc);
	printf("lsb=%d msb=%d\n",read_eeprom(425),read_eeprom(426));

	uv=lsb | (msb<<8);
	if(uv>=0x8000)uv-=0x10000;
	ref=5.000+1.0e-6*uv;
	printf("ref=%g\n",ref);

	return ref;
}

void cal_ni_unknown(void)
{
	comedi_range *range;
	int bipolar_lowgain;
	int bipolar_highgain;
	int unipolar_lowgain;
	int have_ao = 1;

	reset_caldacs();
	printf("Warning: device not calibrated due to insufficient information\n");
	printf("Please send this output to <ds@schleef.org>\n");
	printf("$Id$\n");
	printf("Device name: %s\n",comedi_get_board_name(dev));
	printf("Comedi version: %d.%d.%d\n",
		(comedi_get_version_code(dev)>>16)&0xff,
		(comedi_get_version_code(dev)>>8)&0xff,
		(comedi_get_version_code(dev))&0xff);

	bipolar_lowgain = get_bipolar_lowgain(dev,ad_subdev);
	bipolar_highgain = get_bipolar_highgain(dev,ad_subdev);
	unipolar_lowgain = get_unipolar_lowgain(dev,ad_subdev);

	/* 0 offset, low gain */
	range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
	DPRINT(0,"bipolar zero offset, low gain [%g,%g]\n",
		range->min,range->max);
	channel_dependence(0,bipolar_lowgain);

	/* 0 offset, high gain */
	range = comedi_get_range(dev,ad_subdev,0,bipolar_highgain);
	DPRINT(0,"bipolar zero offset, high gain [%g,%g]\n",
		range->min,range->max);
	channel_dependence(0,bipolar_highgain);

	/* unip/bip offset */
	range = comedi_get_range(dev,ad_subdev,0,unipolar_lowgain);
	DPRINT(0,"unipolar zero offset, low gain [%g,%g]\n",
		range->min,range->max);
	channel_dependence(0,unipolar_lowgain);

	/* voltage reference */
	range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
	DPRINT(0,"bipolar voltage reference, low gain [%g,%g]\n",
		range->min,range->max);
	channel_dependence(5,bipolar_lowgain);

	have_ao = (comedi_get_subdevice_type(dev,da_subdev)==COMEDI_SUBD_AO);
	if(have_ao){
		int ao_chan;

		/* ao 0, zero offset */
		ao_chan = 0;
		set_ao(dev,da_subdev,ao_chan,0,0.0);
		range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
		DPRINT(0,"ao 0, zero offset, low gain [%g,%g]\n",
			range->min,range->max);
		channel_dependence(2,bipolar_lowgain);

		/* ao 0, gain */
		ao_chan = 0;
		set_ao(dev,da_subdev,ao_chan,0,5.0);
		range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
		DPRINT(0,"ao 0, gain, low gain [%g,%g]\n",
			range->min,range->max);
		channel_dependence(6,bipolar_lowgain);

		/* ao 1, zero offset */
		ao_chan = 1;
		set_ao(dev,da_subdev,ao_chan,0,0.0);
		range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
		DPRINT(0,"ao 1, zero offset, low gain [%g,%g]\n",
			range->min,range->max);
		channel_dependence(3,bipolar_lowgain);

		/* ao 1, gain */
		ao_chan = 1;
		set_ao(dev,da_subdev,ao_chan,0,5.0);
		range = comedi_get_range(dev,ad_subdev,0,bipolar_lowgain);
		DPRINT(0,"ao 1, gain, low gain [%g,%g]\n",
			range->min,range->max);
		channel_dependence(7,bipolar_lowgain);
	}

	cal_ni_results();
}

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

void chan_cal(int adc,int cdac,int range,double target)
{
	linear_fit_t l;
	double offset;
	double gain;
	double a;
	char s[32];

	check_gain_chan_x(&l,CR_PACK(adc,range,AREF_OTHER),cdac);
	offset=linear_fit_func_y(&l,caldacs[cdac].current);
	gain=l.slope;
	
	a=caldacs[cdac].current+(target-offset)/gain;

	caldacs[cdac].current=rint(a);
	update_caldac(cdac);

	read_chan2(s,adc,range);
	DPRINT(0,"caldac[%d] set to %g, offset=%s\n",cdac,a,s);
}



void channel_dependence(int adc,int range)
{
	int i;
	double gain;

	for(i=0;i<n_caldacs;i++){
		gain=check_gain_chan(adc,range,i);
	}
}

void caldac_dependence(int caldac)
{
	int i;
	double gain;

	for(i=0;i<8;i++){
		gain=check_gain_chan(i,0,caldac);
	}
}

void dump_curve(int adc,int caldac)
{
	linear_fit_t l;

	check_gain_chan_x(&l,CR_PACK(adc,0,AREF_OTHER),caldac);
}


void setup_caldacs(void)
{
	int n_chan,i;

	if(caldac_subdev<0){
		printf("no calibration subdevice\n");
		return;
	}

	n_chan=comedi_get_n_channels(dev,caldac_subdev);

	for(i=0;i<n_chan;i++){
		caldacs[i].subdev=caldac_subdev;
		caldacs[i].chan=i;
		caldacs[i].maxdata=comedi_get_maxdata(dev,caldac_subdev,i);
		caldacs[i].current=0;
	}

	n_caldacs=n_chan;
}

void reset_caldacs(void)
{
	int i;

	for(i=0;i<n_caldacs;i++){
		caldacs[i].current=caldacs[i].maxdata/2;
		update_caldac(i);
	}
}

void update_caldac(int i)
{
	int ret;
	
	DPRINT(3,"update %d %d %d\n",caldacs[i].subdev,caldacs[i].chan,caldacs[i].current);
	if(caldacs[i].current<0){
		DPRINT(0,"caldac set out of range (%d<0)\n",caldacs[i].current);
		caldacs[i].current=0;
	}
	if(caldacs[i].current>caldacs[i].maxdata){
		DPRINT(0,"caldac set out of range (%d>%d)\n",
			caldacs[i].current,caldacs[i].maxdata);
		caldacs[i].current=caldacs[i].maxdata;
	}

	ret = comedi_data_write(dev,caldacs[i].subdev,caldacs[i].chan,0,0,
		caldacs[i].current);
	if(ret<0)perror("update_caldac()");
}


void check_gain(int ad_chan,int range)
{
	int i;

	for(i=0;i<n_caldacs;i++){
		check_gain_chan(ad_chan,range,i);
	}
}

double check_gain_chan(int ad_chan,int range,int cdac)
{
	linear_fit_t l;

	return check_gain_chan_x(&l,CR_PACK(ad_chan,range,AREF_OTHER),cdac);
}

double check_gain_chan_x(linear_fit_t *l,unsigned int ad_chanspec,int cdac)
{
	int orig,i,n;
	int step;
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;
	char str[20];

	n=caldacs[cdac].maxdata+1;
	memset(l,0,sizeof(*l));

	step=n/256;
	if(step<1)step=1;
	l->n=0;

	l->y_data=malloc(n*sizeof(double)/step);
	if(l->y_data == NULL)
	{
		perror("comedi_calibrate");
		exit(1);
	}

	orig=caldacs[cdac].current;

	new_sv_init(&sv,dev,0,
		CR_CHAN(ad_chanspec),
		CR_RANGE(ad_chanspec),
		CR_AREF(ad_chanspec));

	caldacs[cdac].current=0;
	update_caldac(cdac);

	new_sv_measure(&sv);
	usleep(100000);

	sum_err=0;
	for(i=0;i*step<n;i++){
		caldacs[cdac].current=i*step;
		update_caldac(cdac);

		new_sv_measure(&sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
		l->n++;
	}

	caldacs[cdac].current=orig;
	update_caldac(cdac);

	l->yerr=sum_err/sum_err_count;
	l->dx=step;
	l->x0=0;

	linear_fit_monotonic(l);

	if(verbose>=1 || (verbose>=0 && fabs(l->slope/l->err_slope)>4.0)){
		sci_sprint_alt(str,l->slope,l->err_slope);
		printf("caldac[%d] gain=%s V/bit S_min=%g dof=%g\n",
			cdac,str,l->S_min,l->dof);
		//printf("--> %g\n",fabs(l.slope/l.err_slope));
	}

	if(verbose>=2){
		static int dump_number=0;
		double x,y;

		printf("start dump %d\n",dump_number);
		for(i=0;i<l->n;i++){
			x=l->x0+i*l->dx-l->ave_x;
			y=l->y_data[i];
			printf("D%d: %d %g %g %g\n",dump_number,i,y,
				l->ave_y+l->slope*x,
				l->ave_y+l->slope*x-y);
		}
		printf("end dump\n");
		dump_number++;
	}


	free(l->y_data);

	return l->slope;
}




/* helpers */

int get_bipolar_lowgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double max = 0;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if(range->min != -range->max)continue;
		if(range->max>max){
			ret = i;
			max=range->max;
		}
	}

	return ret;
}

int get_bipolar_highgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double min = HUGE_VAL;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if(range->min != -range->max)continue;
		if(range->max<min){
			ret = i;
			min=range->max;
		}
	}

	return ret;
}

int get_unipolar_lowgain(comedi_t *dev,int subdev)
{
	int ret = -1;
	int i;
	int n_ranges = comedi_get_n_ranges(dev,subdev,0);
	double max = 0;
	comedi_range *range;

	for(i=0;i<n_ranges;i++){
		range = comedi_get_range(dev,subdev,0,i);
		if(range->min != 0)continue;
		if(range->max>max){
			ret = i;
			max=range->max;
		}
	}

	return ret;
}


int read_eeprom(int addr)
{
	unsigned int data=0;

	comedi_data_read(dev,eeprom_subdev,addr,0,0,&data);

	return data;
}

double read_chan(int adc,int range)
{
	int n;
	new_sv_t sv;
	char str[20];

	new_sv_init(&sv,dev,0,adc,range,AREF_OTHER);
	sv.order=7;
	n=new_sv_measure(&sv);

	sci_sprint_alt(str,sv.average,sv.error);
	printf("chan=%d ave=%s\n",adc,str);

	return sv.average;
}

int read_chan2(char *s,int adc,int range)
{
	int n;
	new_sv_t sv;

	new_sv_init(&sv,dev,0,adc,range,AREF_OTHER);
	sv.order=7;
	n=new_sv_measure(&sv);

	return sci_sprint_alt(s,sv.average,sv.error);
}

void set_ao(comedi_t *dev,int subdev,int chan,int range,double value)
{
	comedi_range *r = comedi_get_range(dev,subdev,chan,range);
	lsampl_t maxdata = comedi_get_maxdata(dev,subdev,chan);
	lsampl_t data;

	data = comedi_from_phys(value,r,maxdata);

	comedi_data_write(dev,subdev,chan,range,AREF_GROUND,data);
}


int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,int chan,int range,int aref)
{
	memset(sv,0,sizeof(*sv));

	sv->subd=subdev;
	//sv->t.flags=TRIG_DITHER;
	sv->chan=chan;
	sv->range=range;
	sv->aref=aref;

	//sv->chanlist[0]=CR_PACK(chan,range,aref);

	sv->maxdata=comedi_get_maxdata(dev,subdev,chan);
	sv->rng=comedi_get_range(dev,subdev,chan,range);

	sv->order=7;

	return 0;
}

int comedi_data_read_n(comedi_t *it,unsigned int subdev,unsigned int chan,
	unsigned int range,unsigned int aref,lsampl_t *data,unsigned int n)
{
	comedi_insn insn;
	int ret;

	if(n==0)return 0;

	insn.insn = INSN_READ;
	insn.n = n;
	insn.data = data;
	insn.subdev = subdev;
	insn.chanspec = CR_PACK(chan,range,aref);
	/* enable dithering */
	insn.chanspec |= (1<<26);
	
	ret = comedi_do_insn(it,&insn);

	if(ret>0)return n;

	printf("insn barfed: subdev=%d, chan=%d, range=%d, aref=%d, "
		"n=%d, ret=%d, %s\n",subdev,chan,range,aref,n,ret,
		strerror(errno));

	return ret;
}

int new_sv_measure(new_sv_t *sv)
{
	lsampl_t *data;
	int n,i,ret;

	double a,x,s,s2;

	n=1<<sv->order;

	data=malloc(sizeof(lsampl_t)*n);
	if(data == NULL)
	{
		perror("comedi_calibrate");
		exit(1);
	}

	for(i=0;i<n;){
		ret = comedi_data_read_n(dev,sv->subd,sv->chan,sv->range,
			sv->aref,data+i,n-i);
		if(ret<0){
			printf("barf\n");
			goto out;
		}
		i+=ret;
	}

	s=0;
	s2=0;
	a=comedi_to_phys(data[0],sv->rng,sv->maxdata);
	for(i=0;i<n;i++){
		x=comedi_to_phys(data[i],sv->rng,sv->maxdata);
		s+=x-a;
		s2+=(x-a)*(x-a);
	}
	sv->average=a+s/n;
	sv->stddev=sqrt(n*s2-s*s)/n;
	sv->error=sv->stddev/sqrt(n);

	ret=n;

out:
	free(data);

	return ret;
}

int new_sv_measure_order(new_sv_t *sv,int order)
{
	lsampl_t *data;
	int n,i,ret;
	double a,x,s,s2;

	n=1<<order;

	data=malloc(sizeof(lsampl_t)*n);
	if(data == NULL)
	{
		perror("comedi_calibrate");
		exit(1);
	}

	for(i=0;i<n;){
		ret = comedi_data_read_n(dev,sv->subd,sv->chan,sv->range,
			sv->aref,data+i,n-i);
		if(ret<0){
			printf("barf order\n");
			goto out;
		}
		i+=ret;
	}

	s=0;
	s2=0;
	a=comedi_to_phys(data[0],sv->rng,sv->maxdata);
	for(i=0;i<n;i++){
		x=comedi_to_phys(data[i],sv->rng,sv->maxdata);
		s+=x-a;
		s2+=(x-a)*(x-a);
	}
	sv->average=a+s/n;
	sv->stddev=sqrt(n*s2-s*s)/n;
	sv->error=sv->stddev/sqrt(n);

	ret=n;

out:
	free(data);

	return ret;
}



/* linear fitting */

int calculate_residuals(linear_fit_t *l);

int linear_fit_monotonic(linear_fit_t *l)
{
	double x,y;
	double sxp;
	int i;

	l->min=HUGE_VAL;
	l->max=-HUGE_VAL;
	l->s1=0;
	l->sx=0;
	l->sy=0;
	l->sxy=0;
	l->sxx=0;
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx;
		y=l->y_data[i];

		if(isnan(y))continue;

		if(l->y_data[i]<l->min)l->min=l->y_data[i];
		if(l->y_data[i]>l->max)l->max=l->y_data[i];
		l->s1+=1;
		l->sx+=x;
		l->sy+=y;
		l->sxy+=x*y;
		l->sxx+=x*x;
	}
	sxp=l->sxx-l->sx*l->sx/l->s1;
	
	l->ave_x=l->sx/l->s1;
	l->ave_y=l->sy/l->s1;
	l->slope=(l->s1*l->sxy-l->sx*l->sy)/(l->s1*l->sxx-l->sx*l->sx);
	l->err_slope=l->yerr/sqrt(sxp);
	l->err_ave_y=l->yerr/sqrt(l->s1);

	calculate_residuals(l);

	return 0;
}

int calculate_residuals(linear_fit_t *l)
{
	double x,y;
	double res,sum_res2;
	int i;

	sum_res2=0;
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx-l->ave_x;
		y=l->y_data[i];

		if(isnan(y))continue;

		res=l->ave_y+l->slope*x-y;

		sum_res2+=res*res;
	}
	l->S_min=sum_res2/(l->yerr*l->yerr);
	l->dof=l->s1-2;

	return 0;
}

double linear_fit_func_y(linear_fit_t *l,double x)
{
	return l->ave_y+l->slope*(x-l->ave_x);
}


/* printing of scientific numbers (with errors) */

int sci_sprint(char *s,double x,double y)
{
	int errsig;
	int maxsig;
	int sigfigs;
	double mantissa;
	double error;
	double mindigit;

	errsig = floor(log10(y));
	maxsig = floor(log10(x));
	mindigit = pow(10,errsig);

	if(maxsig<errsig)maxsig=errsig;

	sigfigs = maxsig-errsig+2;

	mantissa = x*pow(10,-maxsig);
	error = y*pow(10,-errsig+1);

	return sprintf(s,"%0.*f(%2.0f)e%d",sigfigs-1,mantissa,error,maxsig);
}

int sci_sprint_alt(char *s,double x,double y)
{
	int errsig;
	int maxsig;
	int sigfigs;
	double mantissa;
	double error;
	double mindigit;

	errsig = floor(log10(fabs(y)));
	maxsig = floor(log10(fabs(x)));
	mindigit = pow(10,errsig);

	if(maxsig<errsig)maxsig=errsig;

	sigfigs = maxsig-errsig+2;

	mantissa = x*pow(10,-maxsig);
	error = y*pow(10,-errsig+1);

	if(isnan(x)){
		return sprintf(s,"%g",x);
	}
	if(errsig==1 && maxsig<4 && maxsig>1){
		return sprintf(s,"%0.0f(%2.0f)",x,error);
	}
	if(maxsig<=0 && maxsig>=-2){
		return sprintf(s,"%0.*f(%2.0f)",sigfigs-1-maxsig,
			mantissa*pow(10,maxsig),error);
	}
	return sprintf(s,"%0.*f(%2.0f)e%d",sigfigs-1,mantissa,error,maxsig);
}

