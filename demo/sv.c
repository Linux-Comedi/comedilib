/*
   Demo of the comedi_sv_*() functions
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

extern int verbose_flag;
extern int subdevice;
extern int range;
extern int channel;
extern int aref;
extern char *filename;

comedi_t *device;


int main(int argc, char *argv[])
{
	lsampl_t data;
	int ret;
	comedi_sv_t sv;
	double volts;

	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	if(verbose_flag){
		printf("measuring device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			filename,subdevice,channel,range,aref);
	}

	comedi_sv_init(&sv,device,subdevice,channel);

	sv.range=range;
	sv.aref=aref;
	sv.n=100;

	ret=comedi_sv_measure(&sv,&volts);
	if(ret<0){
		comedi_perror("comedi_sv_measure()");
		exit(0);
	}

	printf("%g\n",volts);

	return 0;
}

