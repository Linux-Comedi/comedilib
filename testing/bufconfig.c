
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

int test_bufconfig_without_cmd(void);
int test_bufconfig_with_cmd(void);

int test_bufconfig(void)
{
	int flags;

	flags = comedi_get_subdevice_flags(device,subdevice);

	if(flags&SDF_CMD){
		return test_bufconfig_with_cmd();
	}else{
		return test_bufconfig_without_cmd();
	}
}

int test_bufconfig_without_cmd(void)
{
	int ret;

	ret = comedi_get_buffer_size(device,subdevice);
	if(ret<0){
		if(errno==ENODEV){
			printf("got ENODEV, good\n");
		}else{
			printf("E: comedi_get_buffer_size: %s\n",
				strerror(errno));
		}
	}else if(ret==0){
		printf("buffer length is 0, good\n");
	}else{
		printf("E: comedi_get_buffer_size returned %d\n",ret);
	}

	return 0;
}

int test_bufconfig_with_cmd(void)
{
	int ret;
	int len;
	int maxlen;
	int oldlen;

	oldlen = comedi_get_buffer_size(device,subdevice);
	if(oldlen<0){
		printf("E: comedi_get_buffer_size: %s\n",strerror(errno));
	}else{
		printf("buffer size %d\n",oldlen);
	}

	maxlen = comedi_get_max_buffer_size(device,subdevice);
	if(maxlen<0){
		printf("E: comedi_get_max_buffer_size: %s\n",strerror(errno));
	}else{
		printf("max buffer size %d\n",maxlen);
	}

	len=4096;
	printf("setting buffer size to %d\n",len);
	ret = comedi_set_buffer_size(device,subdevice,len);
	if(ret<0){
		printf("E: comedi_set_buffer_size: %s\n",strerror(errno));
	}else{
		printf("buffer size set to %d\n",ret);
	}

	ret = comedi_get_buffer_size(device,subdevice);
	if(ret<0){
		printf("E: comedi_get_buffer_size: %s\n",strerror(errno));
	}else{
		printf("buffer size now at %d\n",ret);
		if(ret != len){
			printf("E: buffer size didn't get set: %d (expected %d)\n",
				ret,len);
		}
	}

	if (maxlen>=0){
		len=maxlen+4096;
		printf("setting buffer size past limit, %d\n",len);
		ret = comedi_set_buffer_size(device,subdevice,len);
		if(ret<0){
			if(errno==EPERM){
				printf("got EPERM, good\n");
			}else{
				printf("E: wrong error comedi_set_buffer_size: %s",
					strerror(errno));
			}
		}else{
			printf("E: comedi_set_buffer_size: didn't get error\n");
		}
	}

	if(maxlen>0){
		len=maxlen;
		printf("setting buffer size to max, %d\n",len);
		ret = comedi_set_buffer_size(device,subdevice,len);
		if(ret<0){
			printf("E: comedi_set_buffer_size: %s\n",strerror(errno));
		}else{
			printf("buffer size now at %d\n",ret);
			if(ret != len){
				printf("E: buffer size didn't get set: %d (expected %d)\n",
					ret,len);
			}
		}
	}

	/*
	 * Don't want to leave buffer set to maximum size unless it was
	 * originally set to the maximum size.
	 */
	len=0;
	if(oldlen>0){
		if(oldlen<maxlen){
			/* Reverting back to old size. */
			len=oldlen;
		}
	}else if(maxlen>4096*10){
		/*
		 * Setting to 10% of maximum size, rounding up to multiple
		 * of 4096 bytes.
		 */
		len=4096-1+maxlen/10;
		len-=len%4096;
	}
	if(len){
		printf("setting buffer size to %d\n",len);
		ret = comedi_set_buffer_size(device,subdevice,len);
		if(ret<0){
			printf("E: comedi_set_buffer_size: %s\n",strerror(errno));
		}else{
			printf("buffer size set to %d\n",ret);
		}
	}else{
		printf("not setting buffer size\n");
	}

	return 0;
}

