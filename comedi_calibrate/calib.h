
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
#define PREOBSERVE_DATA_LEN 10

typedef struct{
	int subdev;
	int chan;

	int maxdata;
	int current;

	int type;
	double gain;
}caldac_t;

typedef struct{
	char *name;

	comedi_insn preobserve_insn;
	lsampl_t preobserve_data[ PREOBSERVE_DATA_LEN ];

	comedi_insn observe_insn;

	//comedi_range *range;
	//int maxdata;
	lsampl_t reference_source;
	double target;
}observable;

typedef struct calibration_setup_struct calibration_setup_t;
struct calibration_setup_struct {
	comedi_t *dev;
	int ad_subdev;
	int da_subdev;
	int eeprom_subdev;
	int caldac_subdev;
	int status;
	unsigned int settling_time_ns;
	observable observables[ N_OBSERVABLES ];
	unsigned int n_observables;
	caldac_t caldacs[ N_CALDACS ];
	unsigned int n_caldacs;
	int (*do_cal) ( calibration_setup_t *setup );
};

extern char *devicename;
extern char *drivername;

extern int verbose;

enum {
	STATUS_UNKNOWN = 0,
	STATUS_GUESS,
	STATUS_SOME,
	STATUS_DONE
};

extern int do_output;

/* high level */

void observe( calibration_setup_t *setup );
int preobserve( calibration_setup_t *setup, int obs);
void observable_dependence( calibration_setup_t *setup, int obs);
void measure_observable( calibration_setup_t *setup, int obs);
void reset_caldacs( calibration_setup_t *setup);

/* drivers */

extern char ni_id[];
extern char cb_id[];

int ni_setup( calibration_setup_t*, const char *device_name );
int cb_setup( calibration_setup_t*, const char *device_name );

/* low level */

void set_target( calibration_setup_t *setup, int obs,double target);
void update_caldac( calibration_setup_t *setup, unsigned int caldac_index );
void setup_caldacs( calibration_setup_t *setup, int caldac_subdev);
void postgain_cal( calibration_setup_t *setup, int obs1, int obs2, int dac);
void cal1( calibration_setup_t *setup, int obs, int dac);
void cal1_fine( calibration_setup_t *setup, int obs, int dac);
void cal_binary( calibration_setup_t *setup, int obs, int dac);
void cal_postgain_binary( calibration_setup_t *setup, int obs1, int obs2, int dac);

/* misc and temp */

void channel_dependence(int adc,int range);
void caldac_dependence(int caldac);
void chan_cal(int adc,int caldac,int range,double target);
int read_eeprom( calibration_setup_t *setup, int addr);

double read_chan( calibration_setup_t *setup, int adc,int range);
int read_chan2( calibration_setup_t *setup, char *s,int adc,int range);
void set_ao(comedi_t *dev,int subdev,int chan,int range,double value);
void check_gain(int ad_chan,int range);
double check_gain_chan(int ad_chan,int range,int cdac);

void cal_ni_results(void);

/* helper functions */

int get_bipolar_lowgain(comedi_t *dev,int subdev);
int get_bipolar_highgain(comedi_t *dev,int subdev);
int get_unipolar_lowgain(comedi_t *dev,int subdev);

/* other */

void comedi_nanodelay(comedi_t *dev, unsigned int delay);

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
double check_gain_chan_x( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac);
double check_gain_chan_fine( calibration_setup_t *setup, linear_fit_t *l,unsigned int ad_chanspec,int cdac);
void dump_curve(linear_fit_t *l);

/* slowly varying measurements */

typedef struct{
	comedi_t *dev;

	int maxdata;
	int order;
	int subd;
	unsigned int chanspec;
	unsigned int settling_time_ns;

	comedi_range *rng;

	int n;
	double average;
	double stddev;
	double error;
}new_sv_t;

int new_sv_measure(comedi_t *dev, new_sv_t *sv);
int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,unsigned int chanspec);

/* saving calibrations to file */
#define CAL_MAX_CHANNELS_LENGTH 128
#define CAL_MAX_RANGES_LENGTH 128
#define CAL_MAX_AREFS_LENGTH 16
typedef struct
{
	unsigned int subdevice;
	caldac_t caldacs[ N_CALDACS ];
	unsigned int caldacs_length;
	/* channels that caldac settings are restricted to */
	int channels[ CAL_MAX_CHANNELS_LENGTH ];
	/* number of elements in channels array, 0 means allow all channels */
	unsigned int channels_length;
	/* ranges that caldac settings are restricted to */
	int ranges[ CAL_MAX_RANGES_LENGTH ];
	/* number of elements in ranges array, 0 means allow all ranges */
	unsigned int ranges_length;
	/* arefs that caldac settings are used restricted to */
	int arefs[ CAL_MAX_AREFS_LENGTH ];
	/* number of elements in arefs array, 0 means allow any aref */
	unsigned int arefs_length;
} saved_calibration_t;

int write_calibration_file( comedi_t *dev, saved_calibration_t settings[],
	unsigned int num_settings );

#endif

