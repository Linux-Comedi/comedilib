
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

static int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd);

#define N_SAMPLES 10000

#define BUFSZ N_SAMPLES*sizeof(sampl_t)

#define MAPLEN 20480

jmp_buf jump_env;

void segv_handler(int num,siginfo_t *si,void *x)
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
	act.sa_sigaction=&segv_handler;
	act.sa_flags = SA_SIGINFO;
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

	setup_segfaulter();

	buf=malloc(BUFSZ);

	if(comedi_get_cmd_fast_1chan(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}

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
			printf("read %d %d\n",ret,total);
		}
	}
	if(memcmp(buf,map,total)){
		printf("E: mmap compare failed\n");
	}else{
		printf("compare ok\n");
	}
	munmap(map,MAPLEN);

	free(buf);

	return 0;
}

static int comedi_get_cmd_src_mask(comedi_t *it,unsigned int s,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(*cmd));

	cmd->subdev = s;

	cmd->flags = 0;

	cmd->start_src = TRIG_ANY;
	cmd->scan_begin_src = TRIG_ANY;
	cmd->convert_src = TRIG_ANY;
	cmd->scan_end_src = TRIG_ANY;
	cmd->stop_src = TRIG_ANY;

	return comedi_command_test(it,cmd);
}


static int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd)
{
	int ret;

	ret = comedi_get_cmd_src_mask(it,s,cmd);
	if(ret<0)return ret;

	cmd->chanlist_len = 1;

	cmd->scan_end_src = TRIG_COUNT;
	cmd->scan_end_arg = 1;

	if(cmd->convert_src&TRIG_TIMER){
		if(cmd->scan_begin_src&TRIG_FOLLOW){
			cmd->convert_src = TRIG_TIMER;
			cmd->scan_begin_src = TRIG_FOLLOW;
		}else{
			cmd->convert_src = TRIG_TIMER;
			cmd->scan_begin_src = TRIG_TIMER;
		}
	}else{
		printf("can't do timed?!?\n");
		return -1;
	}
	if(cmd->stop_src&TRIG_COUNT){
		cmd->stop_src=TRIG_COUNT;
		cmd->stop_arg=2;
	}else if(cmd->stop_src&TRIG_NONE){
		cmd->stop_src=TRIG_NONE;
		cmd->stop_arg=0;
	}else{
		printf("can't find a good stop_src\n");
		return -1;
	}

	ret=comedi_command_test(it,cmd);
	if(ret==3){
		/* good */
		ret=comedi_command_test(it,cmd);
	}
	if(ret==4 || ret==0){
		return 0;
	}
	return -1;
}

