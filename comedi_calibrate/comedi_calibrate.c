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


#define N_CALDACS 16

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
	comedi_trig t;

	unsigned int chanlist[1];

	int maxdata;
	int order;
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
		c = getopt(argc, argv, "rf");
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			fn = argv[optind];
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

	if(!strcmp(drivername,"atmio-E") || !strcmp(drivername,"pcimio-E"))
		cal_ni_mio_E();

	return 0;
}


void cal_ni_mio_E(void)
{
	char *boardname;
	double ref;
	int i;

	boardname=comedi_get_board_name(dev);

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
   4	AO
   5	AO
   6	AO
   7	A0
   8	AO
   9	AO
   10	analog trigger
   11	unknown
 */
		printf("last factory calibration %02d/%02d/%02d\n",
			read_eeprom(508),read_eeprom(507),read_eeprom(506));

		printf("lsb=%d msb=%d\n",read_eeprom(425),read_eeprom(426));

		ref=5.000+(1e-6*(read_eeprom(425)+read_eeprom(426)));
		printf("ref=%g\n",ref);

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

		return;
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

		printf("lsb=%d msb=%d\n",read_eeprom(423),read_eeprom(424));

		ref=5.000+(0.001*(read_eeprom(423)+read_eeprom(424)));
		printf("ref=%g\n",ref);

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
			read_chan(0,i);
		}

		return;
	}
	if(!strcmp(boardname,"pci-mio-16xe-10")){
/*
 * results of channel dependence test:
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

		printf("lsb=%d msb=%d\n",read_eeprom(430),read_eeprom(431));

		ref=5.000+(0.001*(read_eeprom(430)+read_eeprom(431)));
		printf("ref=%g\n",ref);

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

int dump_flag;

void dump_curve(int adc,int caldac)
{
	linear_fit_t l;

	dump_flag=1;
	check_gain_chan_x(&l,adc,0,caldac);
	dump_flag=0;
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
	new_sv_t sv;
	double sum_err;
	int sum_err_count=0;

	n=caldacs[cdac].maxdata+1;
	memset(l,0,sizeof(*l));
	l->y_data=malloc(n*sizeof(double));

	orig=caldacs[cdac].current;

	new_sv_init(&sv,dev,0,ad_chan,range,AREF_OTHER);

	sum_err=0;
	for(i=0;i<n;i++){
		caldacs[cdac].current=i;
		update_caldac(cdac);

		new_sv_measure(&sv);

		l->y_data[i]=sv.average;
		if(!isnan(sv.average)){
			sum_err+=sv.error;
			sum_err_count++;
		}
	}

	caldacs[cdac].current=orig;
	update_caldac(cdac);

	l->yerr=sum_err/sum_err_count;
	l->n=n;
	l->dx=1;
	l->x0=0;

	linear_fit_monotonic(l);

	printf("caldac[%d] gain=%g V/bit err=%g S_min=%g dof=%g\n",
		cdac,l->slope,l->err_slope,l->S_min,l->dof);
	//printf("--> %g\n",fabs(l.slope/l.err_slope));

	if(dump_flag){
		for(i=0;i<n;i++){
			printf("%d %g\n",i,l->y_data[i]);
		}
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
	sv.order=10;
	n=new_sv_measure(&sv);

	printf("chan=%d ave=%g error=%g\n",adc,sv.average,sv.error);

	return sv.average;
}

void write_caldac(comedi_t *dev,int subdev,int addr,int val)
{
	comedi_trig it;
	unsigned int chan=CR_PACK(addr,0,0);
	sampl_t data=val;

	memset(&it,0,sizeof(it));

	it.subdev = subdev;
	it.n_chan = 1;
	it.chanlist = &chan;
	it.data = &data;
	it.n = 1;

	if(comedi_trigger(dev,&it)<0){
		perror("write_caldac");
		exit(0);
	}
}



int new_sv_init(new_sv_t *sv,comedi_t *dev,int subdev,int chan,int range,int aref)
{
	memset(sv,0,sizeof(*sv));

	sv->t.subdev=subdev;
	sv->t.flags=TRIG_DITHER;
	sv->t.n_chan=1;
	sv->t.chanlist=sv->chanlist;

	sv->chanlist[0]=CR_PACK(chan,range,aref);

	sv->maxdata=comedi_get_maxdata(dev,subdev,chan);
	sv->rng=comedi_get_range(dev,subdev,chan,range);

	sv->order=7;

	return 0;
}

int new_sv_measure(new_sv_t *sv)
{
	sampl_t *data;
	int n,i,ret;

	double a,x,s,s2;

	n=1<<sv->order;

	data=malloc(sizeof(sampl_t)*n);

	for(i=0;i<n;){
		sv->t.data=data+i;
		sv->t.n=n-i;
		ret=comedi_trigger(dev,&sv->t);
		if(ret<0)goto out;
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
	sampl_t *data;
	int n,i,ret;
	double a,x,s,s2;

	n=1<<order;

	data=malloc(sizeof(sampl_t)*n);

	for(i=0;i<n;){
		sv->t.data=data+i;
		sv->t.n=n-i;
		ret=comedi_trigger(dev,&sv->t);
		if(ret<0)goto out;
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

	l->s1=0;
	l->sx=0;
	l->sy=0;
	l->sxy=0;
	l->sxx=0;
	for(i=0;i<l->n;i++){
		x=l->x0+i*l->dx;
		y=l->y_data[i];

		if(isnan(y))continue;

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

