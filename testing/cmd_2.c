
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
static int get_chunks_per_length(int length);

#define BUFSZ 10000

int test_cmd_fifo_depth_check(void)
{
	int len;

	for(len=64;len<65536;len<<=1){
		printf("%d, %d\n",len,get_chunks_per_length(len));
	}

	return 0;
}

static int get_chunks_per_length(int length)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	int chunks=0;

	if(comedi_get_cmd_fast_1chan(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = length;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	comedi_command(device,&cmd);

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
		}
	}

	return chunks;
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

