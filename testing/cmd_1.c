
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

int comedi_get_cmd_src_mask(comedi_t *it,unsigned int s,comedi_cmd *cmd);
int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd);
void probe_max_1chan(comedi_t *it,int s);
char *tobinary(char *s,int bits,int n);
char *cmd_src(int src,char *buf);

int test_cmd_probe_src_mask(void)
{
	comedi_cmd cmd;
	char buf[100];
	int ret;

	printf("rev 1\n");

	ret = comedi_get_cmd_src_mask(device,subdevice,&cmd);
	if(ret<0){
		printf("not supported\n");
		return 0;
	}
	printf("command source mask:\n");
	printf("  start: %s\n",cmd_src(cmd.start_src,buf));
	printf("  scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
	printf("  convert: %s\n",cmd_src(cmd.convert_src,buf));
	printf("  scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
	printf("  stop: %s\n",cmd_src(cmd.stop_src,buf));

	return 0;
}

int test_cmd_probe_fast_1chan(void)
{
	comedi_cmd cmd;
	char buf[100];

	printf("command fast 1chan:\n");
	if(comedi_get_cmd_fast_1chan(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}
	printf("  start: %s %d\n",
		cmd_src(cmd.start_src,buf),cmd.start_arg);
	printf("  scan_begin: %s %d\n",
		cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
	printf("  convert: %s %d\n",
		cmd_src(cmd.convert_src,buf),cmd.convert_arg);
	printf("  scan_end: %s %d\n",
		cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
	printf("  stop: %s %d\n",
		cmd_src(cmd.stop_src,buf),cmd.stop_arg);

	return 0;
}

#define BUFSZ 1000

int test_cmd_read_fast_1chan(void)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;

	if(comedi_get_cmd_fast_1chan(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = 10000;
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
			printf("read %d %d\n",ret,total);
		}
	}

	return 0;
}

char *tobinary(char *s,int bits,int n)
{
	int bit=1<<n;
	char *t=s;
	
	for(;bit;bit>>=1)
		*t++=(bits&bit)?'1':'0';
	*t=0;
	
	return s;
}

char *cmd_src(int src,char *buf)
{
	buf[0]=0;

	if(src&TRIG_NONE)strcat(buf,"none|");
	if(src&TRIG_NOW)strcat(buf,"now|");
	if(src&TRIG_FOLLOW)strcat(buf,"follow|");
	if(src&TRIG_TIME)strcat(buf,"time|");
	if(src&TRIG_TIMER)strcat(buf,"timer|");
	if(src&TRIG_COUNT)strcat(buf,"count|");
	if(src&TRIG_EXT)strcat(buf,"ext|");
	if(src&TRIG_INT)strcat(buf,"int|");

	if(strlen(buf)==0){
		sprintf(buf,"unknown(0x%02x)",src);
	}else{
		buf[strlen(buf)-1]=0;
	}

	return buf;
}


int comedi_get_cmd_src_mask(comedi_t *it,unsigned int s,comedi_cmd *cmd)
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


int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd)
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

