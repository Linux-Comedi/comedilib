
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

void probe_max_1chan(comedi_t *it,int s);
char *tobinary(char *s,int bits,int n);
char *cmd_src(int src,char *buf);
int count_bits(unsigned int bits);

int test_cmd_no_cmd(void)
{
	int ret;
	comedi_cmd cmd;

	if(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD){
		printf("not applicable\n");
		return 0;
	}

	ret = comedi_get_cmd_src_mask(device,subdevice,&cmd);
	if(ret<0){
		if(errno == EIO){
			printf("got EIO, good\n");
		}else{
			printf("E: comedi_get_cmd_src_mask: %s\n",
				strerror(errno));
		}
	}else{
		printf("E: comedi_get_cmd_src_mask returned %d\n",ret);
	}

	return 0;
}

int test_cmd_probe_src_mask(void)
{
	comedi_cmd cmd;
	char buf[100];
	int ret;

	if(!(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD)){
		printf("not applicable\n");
		return 0;
	}

	printf("rev 1\n");

	ret = comedi_get_cmd_src_mask(device,subdevice,&cmd);
	if(ret<0){
		printf("E: comedi_get_cmd_src_mask failed %s\n",
			strerror(errno));
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

	if(!(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD)){
		printf("not applicable\n");
		return 0;
	}

	printf("command fast 1chan:\n");
	if(comedi_get_cmd_generic_timed(device,subdevice,&cmd,1)<0){
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

#define BUFSZ 2000

int test_cmd_read_fast_1chan(void)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	unsigned int flags = comedi_get_subdevice_flags(device,subdevice);

	if(!(flags&SDF_CMD) || flags&SDF_WRITEABLE){
		printf("not applicable\n");
		return 0;
	}

	if(comedi_get_cmd_generic_timed(device,subdevice,&cmd,1)<0){
		printf("  not supported\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;
	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = 100000;
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
			if(verbose)printf("read %d %d\n",ret,total);
		}
	}

	return 0;
}

int test_cmd_write_fast_1chan(void)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	unsigned int flags = comedi_get_subdevice_flags(device,subdevice);

	if(!(flags&SDF_CMD) || !(flags&SDF_WRITEABLE)){
		printf("not applicable\n");
		return 0;
	}

	if(comedi_get_cmd_generic_timed(device,subdevice,&cmd,1)<0){
		printf("  not supported\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;
	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = 1000000;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	memset(buf,0,BUFSZ);

	ret = comedi_command(device,&cmd);
	if(ret<0){
		perror("comedi_command");
	}

	go = 1;
	while(go){
		ret = write(comedi_fileno(device), buf, BUFSZ);
		if(ret<0){
			perror("write");
			return 0;
		}
		if(ret<BUFSZ){
			go = 0;
			break;
		}

		total += ret;
		if(verbose)printf("write %d %d\n",ret,total);
	}
	
	{
		comedi_insn insn;
		memset(&insn, 0, sizeof(comedi_insn));
		insn.insn = INSN_INTTRIG;
		insn.subdev = subdevice;
		ret = comedi_do_insn(device, &insn);
		if(ret<0){
			perror("comedi_inttrig");
			return 0;
		}
		if(verbose)printf("inttrig\n");
	}

	go=1;
	while(go){
		ret = write(comedi_fileno(device),buf,BUFSZ);
		if(ret<0){
			if(errno==EAGAIN){
				usleep(10000);
			}else{
				go = 0;
				perror("write");
			}
		}else if(ret==0){
			go = 0;
		}else{
			total += ret;
			if(verbose)printf("write %d %d\n",ret,total);
		}
	}

	return 0;
}

int test_cmd_logic_bug(void)
{
	comedi_cmd cmd;
	int ret;

	if(!(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD)){
		printf("not applicable\n");
		return 0;
	}

	printf("rev 1\n");

	ret = comedi_get_cmd_src_mask(device,subdevice,&cmd);
	if(ret<0){
		printf("E: comedi_get_cmd_src_mask failed\n");
		return 0;
	}

	if(count_bits(cmd.start_src)>1)cmd.start_src=0;
	if(count_bits(cmd.scan_begin_src)>1)cmd.scan_begin_src=0;
	if(count_bits(cmd.convert_src)>1)cmd.convert_src=0;
	if(count_bits(cmd.scan_end_src)>1)cmd.scan_end_src=0;
	if(count_bits(cmd.stop_src)>1)cmd.stop_src=0;

	ret = comedi_command_test(device,&cmd);
	if(ret!=1){
		printf("E: command_test returned %d, expected 1, (allowed src==0)\n",ret);
	}else{
		printf("command_test returned %d, good\n",ret);
	}



	return 0;
}

int count_bits(unsigned int bits)
{
	int ret=0;
	while(bits){
		if(bits&1)ret++;
		bits>>=1;
	}
	return ret;
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


