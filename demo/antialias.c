/*
   A little output demo
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
extern int value;
extern char *filename;

comedi_t *device;

void ao_antialias(unsigned int data);

int main(int argc, char *argv[])
{
	lsampl_t data;
	int ret;

#if 0
	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	data = value; 
	if(verbose_flag){
		printf("writing %d to device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			data,filename,subdevice,channel,range,aref);
	}

	ret=comedi_data_write(device,subdevice,channel,range,aref,data);
	if(ret<0){
		comedi_perror(filename);
		exit(0);
	}

	printf("%d\n",data);
#endif

	ao_antialias((1000<<16)+1000);

	return 0;
}


void ao_antialias(unsigned int data)
{
	unsigned int hibits=data>>16;
	unsigned int lobits=data&0xffff;
	int i;
	unsigned int acc;

	acc=0;
	for(i=0;i<100;i++){
		acc+=lobits;
		printf("%d\n",hibits+(acc>>16));
		acc&=0xffff;
	}
}

