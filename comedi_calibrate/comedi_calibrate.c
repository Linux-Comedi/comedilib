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

/* global variables */

caldac caldacs[N_CALDACS];
int n_caldacs;

observable observables[N_OBSERVABLES];
int n_observables;

comedi_t *dev;

int ad_subdev;
int da_subdev;
int eeprom_subdev;
int caldac_subdev;

char *drivername = NULL;
char *devicename = NULL;

int verbose = 0;

/* */


struct board_struct{
	char *name;
	char *id;
	int (*setup)( calibration_setup *setup, const char *device_name );
};

struct board_struct drivers[] = {
	{ "ni_pcimio",	ni_id,	ni_setup },
	{ "ni_atmio",	ni_id,	ni_setup },
	{ "ni_mio_cs",	ni_id,	ni_setup },
	{ "cb_pcidas64",	cb_id,	cb_setup },
};
#define n_drivers (sizeof(drivers)/sizeof(drivers[0]))

int do_dump = 0;
int do_reset = 1;
int do_calibrate = 1;
int do_results = 0;
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
	{ "output", 0, &do_output, 1 },
	{ "no-output", 0, &do_output, 0 },
	{ 0 },
};

int main(int argc, char *argv[])
{
	char *fn = NULL;
	int c;
	int i;
	struct board_struct *this_board;
	int index;
	int device_status = STATUS_UNKNOWN;
	calibration_setup setup;

	fn = "/dev/comedi0";
	while (1) {
		c = getopt_long(argc, argv, "f:vq", options, &index);
		if (c == -1)break;
		switch (c) {
		case 0:
			continue;
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
			printf("bad option %d\n",c);
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
	memset( &setup, 0, sizeof( setup ) );
	this_board->setup( &setup, devicename );
	device_status = setup.status;

	if(device_status<STATUS_DONE){
		printf("Warning: device not fully calibrated due to insufficient information\n");
		printf("Please send this output to <ds@schleef.org>\n");
		if(verbose<1)verbose=1;
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
		if(device_status==STATUS_GUESS){
			do_reset=1;
			do_dump=1;
			do_calibrate=1;
			do_results=1;
		}
	}
	if(verbose>=0){
		char *s = "$Id$";

		printf("%.*s\n",(int)strlen(s)-2,s+1);
		printf("Driver name: %s\n",drivername);
		printf("Device name: %s\n",devicename);
		printf("%.*s\n",(int)strlen(this_board->id)-2,this_board->id+1);
		printf("Comedi version: %d.%d.%d\n",
			(comedi_get_version_code(dev)>>16)&0xff,
			(comedi_get_version_code(dev)>>8)&0xff,
			(comedi_get_version_code(dev))&0xff);
	}

	if(do_reset)reset_caldacs( &setup );
	if(do_dump) observe();
	if(do_calibrate && do_cal) setup.do_cal( &setup );
	if(do_results) observe();

	return 0;
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
		if(verbose>=1){
			observable_dependence(i);
		}
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
	// read internal calibration source and turn on dithering
	sv.cr_flags = CR_ALT_FILTER | CR_ALT_SOURCE;
	n = new_sv_measure(&sv);

	sci_sprint_alt(s,sv.average,sv.error);
	DPRINT(0,"offset %s, target %g\n",s,observables[obs].target);
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

	DPRINT(0,"postgain: %s; %s\n",observables[obs1].name,
		observables[obs2].name);
	preobserve(obs1);
	check_gain_chan_x(&l,observables[obs1].observe_insn.chanspec,dac);
	offset1=linear_fit_func_y(&l,caldacs[dac].current);
	DPRINT(2,"obs1: [%d] offset %g\n",obs1,offset1);
	range1 = comedi_get_range(dev,observables[obs1].observe_insn.subdev,
		CR_CHAN(observables[obs1].observe_insn.chanspec),
		CR_RANGE(observables[obs1].observe_insn.chanspec));
	slope1=l.slope;

	preobserve(obs2);
	check_gain_chan_x(&l,observables[obs2].observe_insn.chanspec,dac);
	offset2=linear_fit_func_y(&l,caldacs[dac].current);
	DPRINT(2,"obs2: [%d] offset %g\n",obs2,offset2);
	range2 = comedi_get_range(dev,observables[obs2].observe_insn.subdev,
		CR_CHAN(observables[obs2].observe_insn.chanspec),
		CR_RANGE(observables[obs2].observe_insn.chanspec));
	slope2=l.slope;

	gain = (range1->max-range1->min)/(range2->max-range2->min);
	DPRINT(4,"range1 %g range2 %g\n", range1->max-range1->min,
		range2->max-range2->min);
	DPRINT(3,"gain: %g\n",gain);

	DPRINT(3,"difference: %g\n",offset2-offset1);

	a = (offset1-offset2)/(slope1-slope2);
	a=caldacs[dac].current-a;

	caldacs[dac].current=rint(a);
	update_caldac( caldacs[dac] );
	usleep(100000);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);

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
	double a;

	DPRINT(0,"linear: %s\n",observables[obs].name);
	preobserve(obs);
	check_gain_chan_x(&l,observables[obs].observe_insn.chanspec,dac);
	a=linear_fit_func_x(&l,observables[obs].target);

	caldacs[dac].current=rint(a);
	update_caldac( caldacs[dac] );
	usleep(100000);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);
	if(verbose>=3){
		measure_observable(obs);
	}
}

void cal1_fine(int obs, int dac)
{
	linear_fit_t l;
	double a;

	DPRINT(0,"linear fine: %s\n",observables[obs].name);
	preobserve(obs);
	check_gain_chan_fine(&l,observables[obs].observe_insn.chanspec,dac);
	a=linear_fit_func_x(&l,observables[obs].target);

	caldacs[dac].current=rint(a);
	update_caldac( caldacs[dac] );
	usleep(100000);

	DPRINT(0,"caldac[%d] set to %g (%g)\n",dac,rint(a),a);
	if(verbose>=3){
		measure_observable(obs);
	}
}

#if 0
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
	DPRINT(1,"caldac[%d] set to %g, offset=%s\n",cdac,a,s);
}
#endif


#if 0
void channel_dependence(int adc,int range)
{
	int i;
	double gain;

	for(i=0;i<n_caldacs;i++){
		gain=check_gain_chan(adc,range,i);
	}
}
#endif

#if 0
void caldac_dependence(int caldac)
{
	int i;
	double gain;

	for(i=0;i<8;i++){
		gain=check_gain_chan(i,0,caldac);
	}
}
#endif


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

void reset_caldacs( const calibration_setup *setup )
{
	int i;

	for( i = 0; i < setup->n_caldacs; i++){
		setup->caldacs[i].current = setup->caldacs[i].maxdata / 2;
		update_caldac( setup->caldacs[i] );
	}
}

void update_caldac( caldac dac )
{
	int ret;

	DPRINT(4,"update %d %d %d\n", dac.subdev, dac.chan, dac.current);
	if( dac.current < 0 ){
		DPRINT(1,"caldac set out of range (%d<0)\n", dac.current);
		dac.current = 0;
	}
	if( dac.current > dac.maxdata ){
		DPRINT(1,"caldac set out of range (%d>%d)\n",
			dac.current, dac.maxdata);
		dac.current = dac.maxdata;
	}

	ret = comedi_data_write(dev, dac.subdev, dac.chan, 0, 0,
		dac.current);
	if(ret < 0) perror("update_caldac()");
}

#if 0
void check_gain(int ad_chan,int range)
{
	int i;

	for(i=0;i<n_caldacs;i++){
		check_gain_chan(ad_chan,range,i);
	}
}
#endif

#if 0
double check_gain_chan(int ad_chan,int range,int cdac)
{
	linear_fit_t l;

	return check_gain_chan_x(&l,CR_PACK(ad_chan,range,AREF_OTHER),cdac);
}
#endif


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
	// read internal calibration source and turn on dithering
	sv.cr_flags = CR_ALT_FILTER | CR_ALT_SOURCE;

	caldacs[cdac].current=0;
	update_caldac( caldacs[cdac] );
	usleep(100000);

	new_sv_measure(&sv);

	sum_err=0;
	for(i=0;i*step<n;i++){
		caldacs[cdac].current=i*step;
		update_caldac( caldacs[cdac] );
		//usleep(100000);

		new_sv_measure(&sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
		l->n++;
	}

	caldacs[cdac].current=orig;
	update_caldac( caldacs[cdac] );

	l->yerr=sum_err/sum_err_count;
	l->dx=step;
	l->x0=0;

	linear_fit_monotonic(l);

	if(verbose>=2 || (verbose>=1 && fabs(l->slope/l->err_slope)>4.0)){
		sci_sprint_alt(str,l->slope,l->err_slope);
		printf("caldac[%d] gain=%s V/bit S_min=%g dof=%g\n",
			cdac,str,l->S_min,l->dof);
		//printf("--> %g\n",fabs(l.slope/l.err_slope));
	}

	if(verbose>=3)dump_curve(l);

	free(l->y_data);

	return l->slope;
}


double check_gain_chan_fine(linear_fit_t *l,unsigned int ad_chanspec,int cdac)
{
	int orig,i,n;
	int step;
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;
	char str[20];
	int fine_size = 10;

	n=2*fine_size+1;
	memset(l,0,sizeof(*l));

	step=1;
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
	// read internal calibration source and turn on dithering
	sv.cr_flags = CR_ALT_FILTER | CR_ALT_SOURCE;

	caldacs[cdac].current=0;
	update_caldac( caldacs[cdac] );
	usleep(100000);

	new_sv_measure(&sv);

	sum_err=0;
	for(i=0;i<n;i++){
		caldacs[cdac].current=i+orig-fine_size;
		update_caldac( caldacs[cdac] );
		usleep(100000);

		new_sv_measure(&sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
		l->n++;
	}

	caldacs[cdac].current=orig;
	update_caldac( caldacs[cdac] );

	l->yerr=sum_err/sum_err_count;
	l->dx=1;
	l->x0=orig-fine_size;

	linear_fit_monotonic(l);

	if(verbose>=2 || (verbose>=1 && fabs(l->slope/l->err_slope)>4.0)){
		sci_sprint_alt(str,l->slope,l->err_slope);
		printf("caldac[%d] gain=%s V/bit S_min=%g dof=%g\n",
			cdac,str,l->S_min,l->dof);
		//printf("--> %g\n",fabs(l.slope/l.err_slope));
	}

	if(verbose>=3)dump_curve(l);

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
		/* This method is better than a direct test, which might fail */
		if((range->min+range->max)>(range->max*0.0001))continue;
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
		/* This method is better than a direct test, which might fail */
		if((range->min+range->max)>(range->max*0.0001))continue;
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
	sv.cr_flags = CR_ALT_FILTER;

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
	// turn on dithering
	sv.cr_flags = CR_ALT_FILTER;

	n=new_sv_measure(&sv);

	return sci_sprint_alt(s,sv.average,sv.error);
}

#if 0
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value)
{
	comedi_range *r = comedi_get_range(dev,subdev,chan,range);
	lsampl_t maxdata = comedi_get_maxdata(dev,subdev,chan);
	lsampl_t data;

	data = comedi_from_phys(value,r,maxdata);

	comedi_data_write(dev,subdev,chan,range,AREF_GROUND,data);
}
#endif


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

	ret = comedi_data_read_n(dev, sv->subd, sv->chan | sv->cr_flags, sv->range,
		sv->aref, data, n);
	if(ret<0){
		printf("barf\n");
		goto out;
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

	ret = comedi_data_read_n(dev, sv->subd, sv->chan | sv->cr_flags, sv->range,
		sv->aref, data, n);
	if(ret<0){
		printf("barf order\n");
		goto out;
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

double linear_fit_func_x(linear_fit_t *l,double y)
{
	return l->ave_x+(y-l->ave_y)/l->slope;
}

void dump_curve(linear_fit_t *l)
{
	static int dump_number=0;
	double x,y;
	int i;

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

