
#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "comedi_test.h"


int test_bufconfig(void)
{
	int ret;
	int len;
	int maxlen;

	printf("joe\n");

	ret = comedi_get_buffer_size(device,subdevice);
	if(ret<0){
		perror("comedi_get_buffer_size");
	}else{
		printf("buffer size %d\n",ret);
	}

	maxlen = comedi_get_max_buffer_size(device,subdevice);
	if(maxlen<0){
		perror("comedi_get_max_buffer_size");
	}else{
		printf("max buffer size %d\n",maxlen);
	}

	len=4096;
	printf("setting buffer size to %d\n",len);
	ret = comedi_set_buffer_size(device,subdevice,len);
	if(ret<0){
		perror("comedi_set_buffer_size");
	}else{
		printf("buffer size set to %d\n",ret);
	}

	ret = comedi_get_buffer_size(device,subdevice);
	if(ret<0){
		perror("comedi_get_buffer_size");
	}else{
		printf("buffer size now at %d\n",ret);
	}

	len=maxlen+4096;
	printf("setting buffer size past limit, %d\n",len);
	ret = comedi_set_buffer_size(device,subdevice,len);
	if(ret<0){
		perror("comedi_set_buffer_size");
	}else{
		printf("buffer size now at %d\n",ret);
	}

	len=maxlen;
	printf("setting buffer size to max, %d\n",len);
	ret = comedi_set_buffer_size(device,subdevice,len);
	if(ret<0){
		perror("comedi_set_buffer_size");
	}else{
		printf("buffer size now at %d\n",ret);
	}


	return 0;
}

