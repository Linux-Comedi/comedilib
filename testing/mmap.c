
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
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#include "comedi_test.h"

/* XXX this should come from elsewhere */
#define PAGE_SIZE 4096

#define N_SAMPLES 10000

#define BUFSZ N_SAMPLES*sizeof(sampl_t)

#define MAPLEN 20480

jmp_buf jump_env;

void segv_handler(int num)
{
	longjmp(jump_env,1);
}

int test_segfault(void *memptr)
{
	volatile char tmp;
	int ret;

	ret=setjmp(jump_env);
	if(!ret) tmp = *((char *)(memptr));
	return ret;
}

void setup_segfaulter(void)
{
	struct sigaction act;

	memset(&act,0,sizeof(act));
	act.sa_handler=&segv_handler;
	sigaction(SIGSEGV,&act,NULL);
}

int test_mmap(void)
{
	comedi_cmd cmd;
	char *buf;
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	void *b, *adr;
	sampl_t *map;

	if(!(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD)){
		printf("not applicable\n");
		return 0;
	}

	if(comedi_get_cmd_generic_timed(device,subdevice,&cmd,1)<0){
		printf("E: comedi_get_cmd_generic_timed failed\n");
		return 0;
	}

	setup_segfaulter();

	buf=malloc(BUFSZ);

	map=mmap(NULL,MAPLEN,PROT_READ,MAP_SHARED,comedi_fileno(device),0);
	if(!map){
		printf("E: mmap() failed\n");
		return 0;
	}

	/* test readability */
	for(adr=map;adr<(void *)map+MAPLEN;adr+=PAGE_SIZE){
		ret=test_segfault(adr);
		if(ret){
			printf("E: %p failed\n",adr);
		}else{
			printf("%p ok\n",adr);
		}
	}

	if(realtime)cmd.flags |= TRIG_RT;

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = N_SAMPLES;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	comedi_command(device,&cmd);

	go=1;
	b=buf;
	while(go){
		ret = read(comedi_fileno(device),b,BUFSZ);
		if(ret<0){
			if(errno==EAGAIN){
				usleep(10000);
			}else{
				go = 0;
				perror("read");
			}
		}else if(ret==0){
			go = 0;
		}else{
			total += ret;
			b += ret;
			if(verbose) printf("read %d %d\n",ret,total);
		}
	}
	if(memcmp(buf,map,total)){
		printf("E: mmap compare failed\n");
	}else{
		printf("compare ok\n");
	}
	munmap(map,MAPLEN);

	/* test if area is really unmapped */
	for(adr=map;adr<(void *)map+MAPLEN;adr+=PAGE_SIZE){
		ret=test_segfault(adr);
		if(ret){
			printf("%p segfaulted (ok)\n",adr);
		}else{
			printf("E: %p still mapped\n",adr);
		}
	}

	free(buf);

	return 0;
}

