
#ifndef __CALIB_H_
#define __CALIB_H_

#include <comedilib.h>
#if 0
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#endif

#define DPRINT(level,fmt,args...) do{if(verbose>=level)printf(fmt, ## args);}while(0)

#define N_CALDACS 64
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

extern caldac caldacs[N_CALDACS];
extern int n_caldacs;

extern observable observables[N_OBSERVABLES];
extern int n_observables;

extern comedi_t *dev;

extern int ad_subdev;
extern int da_subdev;
extern int eeprom_subdev;
extern int caldac_subdev;

extern char *devicename;
extern char *drivername;

extern int verbose;

enum {
	STATUS_UNKNOWN = 0,
	STATUS_GUESS,
	STATUS_SOME,
	STATUS_DONE
};
extern int device_status;

extern int do_output;

/* high level */

void observe(void);
void preobserve(int obs);
void observable_dependence(int obs);
void measure_observable(int obs);
void reset_caldacs(void);

/* drivers */

extern char ni_id[];

int ni_setup(void);

/* low level */

void set_target(int obs,double target);
void update_caldac(int i);
void setup_caldacs(void);
void postgain_cal(int obs1, int obs2, int dac);
void cal1(int obs, int dac);
void cal1_fine(int obs, int dac);

/* misc and temp */

void channel_dependence(int adc,int range);
void caldac_dependence(int caldac);
void chan_cal(int adc,int caldac,int range,double target);
int read_eeprom(int addr);

double read_chan(int adc,int range);
int read_chan2(char *s,int adc,int range);
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value);
void check_gain(int ad_chan,int range);
double check_gain_chan(int ad_chan,int range,int cdac);

void (*do_cal)(void);
void cal_ni_results(void);

/* helper functions */

int get_bipolar_lowgain(comedi_t *dev,int subdev);
int get_bipolar_highgain(comedi_t *dev,int subdev);
int get_unipolar_lowgain(comedi_t *dev,int subdev);

/* printing scientific numbers */

int sci_sprint(char *s,double x,double y);
int sci_sprint_alt(char *s,double x,double y);

/* linear fitting */

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
double linear_fit_func_x(linear_fit_t *l,double y);
double check_gain_chan_x(linear_fit_t *l,unsigned int ad_chanspec,int cdac);
double check_gain_chan_fine(linear_fit_t *l,unsigned int ad_chanspec,int cdac);
void dump_curve(linear_fit_t *l);

/* slowly varying measurements */

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


#endif

