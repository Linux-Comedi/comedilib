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


#define N_CALDACS 32

typedef struct{
	int subdev;
	int chan;

	int maxdata;
	int current;

	int type;
	double gain;
}caldac;

static caldac caldacs[N_CALDACS];
static int n_caldacs;
static comedi_t *dev;

static int ad_subdev;
static int eeprom_subdev;

double read_chan(int adc,int range);
void write_caldac(comedi_t *dev,int subdev,int addr,int val);
void check_gain(int ad_chan,int range);
double check_gain_chan(int ad_chan,int range,int cdac);

int verbose = 1;


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
double check_gain_chan_x(linear_fit_t *l,int ad_chan,int range,int cdac);

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

int main(int argc, char *argv[])
{
	char *fn = NULL;
	int c;
	char *drivername;


	fn = "/dev/comedi0";
	while (1) {
		c = getopt(argc, argv, "f:v");
		if (c == -1)
			break;
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

	ad_subdev=0;
	setup_caldacs();

	eeprom_subdev=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_MEMORY,0);

	drivername=comedi_get_driver_name(dev);

	if(   !strcmp(drivername,"atmio-E")
	   || !strcmp(drivername,"ni_atmio") 
	   || !strcmp(drivername,"pcimio-E") 
	   || !strcmp(drivername,"ni_pcimio") 
	   || !strcmp(drivername,"ni_mio_cs"))
		cal_ni_mio_E();

	return 0;
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

void cal_ni_mio_E(void)
{
	char *boardname;
	double ref;
	int i;

	boardname=comedi_get_board_name(dev);

	reset_caldacs();

	if(!strcmp(boardname,"at-mio-16e-1") ||
	   !strcmp(boardname,"at-mio-16e-2") ||
	   !strcmp(boardname,"at-mio-64e-3") ||
	   !strcmp(boardname,"pci-mio-16e-1")){
/*
   layout

   0	AI pre-gain offset	1.5e-6
   1	AI post-gain offset	8.1e-4
   2	AI unipolar offset	7.9e-4
   3	AI gain			
   4	AO 0 	-1.2e-4		-1.2e-4
   5	AO 0 	-8.0e-4		-8.0e-4
   6	AO 0 	1.9e-4		-3.8e-7
   7	AO 1	-8.0e-5		-1.2e-4
   8	AO 1	-7.9e-4		-7.9e-4
   9	AO 1	1.9e-4		3.0e-7
   10	analog trigger
   11	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(425,426);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal();

		printf("pregain offset\n");
		chan_cal(0,0,7,0.0);
		chan_cal(0,0,7,0.0);

		printf("unipolar offset\n");
		chan_cal(0,2,8,0.0);
		chan_cal(0,2,8,0.0);

		printf("gain offset\n");
		chan_cal(5,3,0,5.0);
		chan_cal(5,3,0,5.0);

		printf("ao 0 offset\n");
		comedi_data_write(dev,1,0,0,0,2048);
		chan_cal(2,4,0,0.0);
		chan_cal(2,5,0,0.0);

		printf("ao 0 gain\n");
		comedi_data_write(dev,1,0,0,0,3072);
		chan_cal(6,6,0,0.0);
		chan_cal(6,6,0,0.0);
		comedi_data_write(dev,1,0,0,0,2048);

		//return;
	}
	if(!strcmp(boardname,"at-mio-16e-10")){
/*
   layout

   0	AI pre-gain offset	1.5e-6
   1	AI post-gain offset	8.1e-4
   2	AI unipolar offset	7.9e-4
   3	AI gain			3.5e-4
   4	AO
   5	AO
   6	AO
   7	AO
   8	AO
   9	AO
   10	AI pre-gain offset	6.4e-5
   11	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(423,424);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal();

		printf("pregain offset\n");
		chan_cal(0,10,7,0.0);
		chan_cal(0,0,7,0.0);
		chan_cal(0,0,7,0.0);

		printf("unipolar offset\n");
		chan_cal(0,2,8,0.0);
		chan_cal(0,2,8,0.0);

		printf("gain offset\n");
		chan_cal(5,3,0,5.0);
		chan_cal(5,3,0,5.0);

		printf("results (offset)\n");
		for(i=0;i<16;i++){
			read_chan(i,0);
		}

		return;
	}
	if(!strcmp(boardname,"pci-mio-16xe-10")){
/*
 * results of channel dependency test:
 *
 * 		[0]	[1]	[2]	[3]	[8]
 * offset, lo			1.9e-4*	2.2e-6	2.4e-7
 * offset, hi			2.0e-6*	2.1e-8	2.7e-7
 * offset, unip			1.9e-4	2.1e-6	3.9e-7
 * ref		-2.3e-5*-1.3e-6*1.9e-4*	2.1e-6* 3.2e-7
 *
 * thus, 2,3 are postgain offset, 8 is pregain, and
 * 0,1 is gain.  Note the suspicious lack of unipolar
 * offset.
 * 
 * layout
 *
 * 0	AI gain			-2.3e-5
 * 1	AI gain			-1.3e-6
 * 2	AI postgain offset	1.9e-4
 * 3	AI postgain offset	2.2e-6
 * 4	AO
 * 5	AO
 * 6	AO
 * 7	AO
 * 8	AI pregain offset	2.4e-7
 * 9	unknown
 * 10	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(430,431);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal_2(0,2,0,6,100.0);
		ni_mio_ai_postgain_cal_2(0,3,0,6,100.0);

		printf("pregain offset\n");
		chan_cal(0,8,6,0.0);
		chan_cal(0,8,6,0.0);

		//printf("unipolar offset\n");
		//chan_cal(0,2,8,0.0);
		//chan_cal(0,2,8,0.0);

		printf("gain offset\n");
		chan_cal(5,0,0,5.0);
		chan_cal(5,1,0,5.0);
		chan_cal(5,1,0,5.0);

		printf("results (offset)\n");
		for(i=0;i<16;i++){
			read_chan(i,0);
		}

		return;
	}
	if(!strcmp(boardname,"DAQCard-ai-16xe-50")){
/*
 * results of channel dependency test:
 *
 * 		[0]	[1]	[2]	[3]	[8]
 * offset, lo	-2.2e-6		1.5e-4*		2.5e-7
 * offset, hi			7.8e-7*		1.3e-7
 * offset, unip	7.4e-4	1.1e-5	1.5e-4		5.5e-7
 * ref		-3.7e-4	-5.4e-6	1.5e-4*		5.5e-7
 *
 * thus, 2 is postgain offset, 8 is pregain, 0 is
 * unipolar offset, 1 is gain
 * 
 * layout
 *
 * 0	AI unipolar offset	7.4e-4
 * 1	AI gain			-5.4e-6
 * 2	AI postgain offset	1.5e-4
 * 3	unknown
 * 4	AO
 * 5	AO
 * 6	AO
 * 7	AO
 * 8	AI pregain offset	2.5e-7
 * 9	unknown
 * 10	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(446,447);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal_2(0,2,0,3,100.0);

		printf("pregain offset\n");
		chan_cal(0,8,3,0.0);
		chan_cal(0,8,3,0.0);

		printf("unipolar offset\n");
		chan_cal(0,0,4,0.0);
		chan_cal(0,0,4,0.0);

		printf("gain offset\n");
		chan_cal(5,1,0,ref);
		chan_cal(5,1,0,ref);

		printf("results (offset)\n");
		for(i=0;i<8;i++){
			read_chan(0,i);
		}

		return;
	}
	if(!strcmp(boardname,"pci-mio-16xe-50")){
/*
 * results of channel dependency test:
 *
 * 		[0]	[1]	[2]	[3]	[8]
 * offset, lo			1.6e-5		2.0e-7
 * offset, hi			1.6e-7		1.8e-7
 * offset, unip	
 * ref		-4.5e-5	-2.9e-6	1.6e-5*		5.5e-7
 *
 * thus, 2 is postgain offset, 8 is pregain, 0 is
 * unipolar offset, 1 is gain
 * 
 * layout
 *
 * 0	AI unipolar offset	7.4e-4
 * 1	AI gain			-5.4e-6
 * 2	AI postgain offset	1.5e-4
 * 3	unknown
 * 4	AO
 * 5	AO
 * 6	AO
 * 7	AO
 * 8	AI pregain offset	2.5e-7
 * 9	unknown
 * 10	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(437,438);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal_2(0,2,0,3,100.0);

		printf("pregain offset\n");
		chan_cal(0,8,3,0.0);
		chan_cal(0,8,3,0.0);

#if 0
		printf("unipolar offset\n");
		chan_cal(0,0,4,0.0);
		chan_cal(0,0,4,0.0);
#endif

		printf("gain offset\n");
		chan_cal(5,0,0,5.0);
		chan_cal(5,1,0,5.0);
		chan_cal(5,1,0,5.0);

		printf("results (offset)\n");
		for(i=0;i<16;i++){
			read_chan(0,i);
		}

		return;
	}
	if(!strcmp(boardname,"pci-6023e")){
/*
 * results of channel dependency test:
 *
 * 		[0]	[1]	[3]	[10]
 * offset, lo	-2.8e-9	-7.6e-4		
 * offset, hi	-2.0e-6	-3.8e-6	-1.4e-6
 * offset, unip		1.0e-1*		
 * ref		-7.6e-7	-7.6e-4	-5.6e-4	-6.2e-8
 * ref2		-6.3e-8	-7.5e-4	-5.6e-4	-1.5e-8
 *
 * 0 is pregain offset
 * 1 is postgain offset
 * 3 is gain
 * 
 * layout
 *
 * 0	AI pregain offset	-2.0e-6
 * 1	AI postgain offset	-7.6e-4
 * 2	unknown
 * 3	AI gain			-5.6e-4
 * 4	AO
 * 5	AO
 * 6	AO
 * 7	AO
 * 8	unknown
 * 9	unknown
 * 10	AI			?
 * 11	unknown
 */
		int offset_ad = 0;
		//int unipolar_offset_ad = 1;
		int gain_ad = 5;
		int pregain_offset_dac = 0;
		int postgain_offset_dac = 1;
		int gain_dac = 3;

		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		ref=ni_get_reference(444,443);

		reset_caldacs();

		printf("postgain offset\n");
		ni_mio_ai_postgain_cal_2(offset_ad,postgain_offset_dac,0,3,200.0);

		printf("pregain offset\n");
		chan_cal(offset_ad,pregain_offset_dac,3,0.0);
		chan_cal(offset_ad,pregain_offset_dac,3,0.0);

		printf("gain offset\n");
		chan_cal(gain_ad,gain_dac,0,5.0);
		chan_cal(gain_ad,gain_dac,0,5.0);

		printf("results (offset)\n");
		for(i=0;i<16;i++){
			read_chan(0,i);
		}

		//return;
	}

	{
		int n_ranges;

	reset_caldacs();
	printf("please send this output to <ds@stm.lbl.gov>\n");
	printf("%s\n",comedi_get_board_name(dev));

	n_ranges=comedi_get_n_ranges(dev,ad_subdev,0);

	/* 0 offset, low gain */
	printf("channel dependence 0 range 0\n");
	channel_dependence(0,0);

	/* 0 offset, high gain */
	printf("channel dependence 0 range %d\n",n_ranges/2-1);
	channel_dependence(0,n_ranges/2-1);

	/* unip/bip offset */
	printf("channel dependence 0 range %d\n",n_ranges/2);
	channel_dependence(0,n_ranges/2);

	/* voltage reference */
	printf("channel dependence 5 range 0\n");
	channel_dependence(5,0);

	}
#if 0
	{
		int n_ranges;

	printf("please send this output to <ds@schleef.org>\n");
	printf("%s\n",comedi_get_board_name(dev));

	n_ranges=comedi_get_n_ranges(dev,ad_subdev,0);

	comedi_data_write(dev,1,0,0,0,2048);
	/* ao0 offset */
	printf("channel dependence ao0=0 range 0\n");
	channel_dependence(2,0);

	comedi_data_write(dev,1,0,0,0,3072);
	/* ao gain */
	printf("channel dependence ao0=5V range 0\n");
	channel_dependence(6,0);

	comedi_data_write(dev,1,0,0,0,2048);

	comedi_data_write(dev,1,1,0,0,2048);
	/* ao0 offset */
	printf("channel dependence ao1=0 range 0\n");
	channel_dependence(3,0);

	comedi_data_write(dev,1,1,0,0,3072);
	/* ao gain */
	printf("channel dependence ao1=5V range 0\n");
	channel_dependence(7,0);
	comedi_data_write(dev,1,1,0,0,2048);

	}
#endif
	
}


void ni_mio_ai_postgain_cal(void)
{
	linear_fit_t l;
	double offset_r0;
	double offset_r7;
	double gain;
	double a;

	check_gain_chan_x(&l,0,0,1);
	offset_r0=linear_fit_func_y(&l,caldacs[1].current);
	printf("offset r0 %g\n",offset_r0);

	check_gain_chan_x(&l,0,7,1);
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

	check_gain_chan_x(&l,chan,range_lo,dac);
	offset_lo=linear_fit_func_y(&l,caldacs[dac].current);
	printf("offset lo %g\n",offset_lo);

	check_gain_chan_x(&l,chan,range_hi,dac);
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

	check_gain_chan_x(&l,adc,range,cdac);
	offset=linear_fit_func_y(&l,caldacs[cdac].current);
	printf("offset %g\n",offset);
	gain=l.slope;
	
	a=caldacs[cdac].current+(target-offset)/gain;
	printf("%g\n",a);

	caldacs[cdac].current=rint(a);
	update_caldac(cdac);
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

	check_gain_chan_x(&l,adc,0,caldac);
}


void setup_caldacs(void)
{
	int s,n_chan,i;

	s=comedi_find_subdevice_by_type(dev,COMEDI_SUBD_CALIB,0);
	if(s<0){
		printf("no calibration subdevice\n");
		return;
	}
	//printf("calibration subdevice is %d\n",s);

	n_chan=comedi_get_n_channels(dev,s);

	for(i=0;i<n_chan;i++){
		caldacs[i].subdev=s;
		caldacs[i].chan=i;
		caldacs[i].maxdata=comedi_get_maxdata(dev,s,i);
		caldacs[i].current=0;
		//printf("caldac %d, %d\n",i,caldacs[i].maxdata);
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
	
	//printf("update %d %d %d\n",caldacs[i].subdev,caldacs[i].chan,caldacs[i].current);
	//ret=comedi_data_write(dev,caldacs[i].subdev,caldacs[i].chan,0,0,caldacs[i].current);
	write_caldac(dev,caldacs[i].subdev,caldacs[i].chan,caldacs[i].current);
	ret=0;
	if(ret<0)
		printf("comedi_data_write() error\n");
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

	return check_gain_chan_x(&l,ad_chan,range,cdac);
}

double check_gain_chan_x(linear_fit_t *l,int ad_chan,int range,int cdac)
{
	int orig,i,n;
	int step;
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;

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

	new_sv_init(&sv,dev,0,ad_chan,range,AREF_OTHER);

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

	printf("caldac[%d] gain=%g V/bit err=%g S_min=%g dof=%g min=%g max=%g\n",
		cdac,l->slope,l->err_slope,l->S_min,l->dof,l->min,l->max);
	//printf("--> %g\n",fabs(l.slope/l.err_slope));

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

	new_sv_init(&sv,dev,0,adc,range,AREF_OTHER);
	sv.order=7;
	n=new_sv_measure(&sv);

	printf("chan=%d ave=%g error=%g\n",adc,sv.average,sv.error);

	return sv.average;
}

void write_caldac(comedi_t *dev,int subdev,int addr,int val)
{
	comedi_data_write(dev,subdev,addr,0,0,val);
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
	
	ret = comedi_do_insn(it,&insn);

	/* comedi_do_insn returns the number of sucessful insns.
	 * Hopefully 1. */
	if(ret==1)return n;

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

