
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

static int comedi_get_cmd_src_mask(comedi_t *it,unsigned int s,comedi_cmd *cmd);
static int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd);
static int do_continuous(int multiplier);

#define BUFSZ 10000

int test_cmd_continuous(void)
{
	int mult;

	/* as if doing _one_ infinite loop wasn't slow enough,
	 * we loop through with higher and higher multipliers,
	 * in case the test fails because of latency problems */

	for(mult=1;mult<1024;mult*=2){
		do_continuous(mult);
	}

	return 0;
}

static int do_continuous(int multiplier)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	int chunks=0;
	unsigned long total_secs = 0;
	struct timeval tv,start_tv;

	if(comedi_get_cmd_fast_1chan(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	// slow down a bit
	cmd.scan_begin_arg *= multiplier;
	printf("multiplier=%d, scan_begin_arg=%d\n",
		multiplier,
		cmd.scan_begin_arg);

	ret=comedi_command(device,&cmd);
	if(ret<0){
		perror("comedi_command");
	}else{
		printf("ret==%d\n",ret);
	}

	gettimeofday(&start_tv,NULL);

	go=1;
	while(go){
		ret = read(comedi_fileno(device),buf,BUFSZ);
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
			chunks++;

			gettimeofday(&tv,NULL);
			tv.tv_sec-=start_tv.tv_sec;
			tv.tv_usec-=start_tv.tv_usec;
			if(tv.tv_usec<0){
				tv.tv_usec+=1000000;
				tv.tv_sec--;
			}
			if(tv.tv_sec>total_secs){
				double t;

				t=tv.tv_sec+1e-6*tv.tv_usec;
				printf("%0.3f %d (%g) %d (%g)\n",
					t,
					chunks,chunks/t,
					total,total/t);
				total_secs++;
			}
		}
	}

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

