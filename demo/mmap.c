/*
 * Example of using commands - asynchronous input
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * An example for directly using Comedi commands.  Comedi commands
 * are used for asynchronous acquisition, with the timing controlled
 * by on-board timers or external events.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "examples.h"

#define N_SCANS		10
#define N_CHANS		16

int subdevice = 0;
int chan=0;
int range = 0;
int aref = AREF_GROUND;
double freq = 1000;

#define BUFSZ 1000
char buf[BUFSZ];

sampl_t *map;

void do_cmd_1(comedi_t *dev);
void do_cmd_2(comedi_t *dev);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);
void dump_cmd(comedi_cmd *cmd);

int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_t *dev;

	fn = "/dev/comedi0";

	dev = comedi_open(fn);
	if(!dev){
		perror(fn);
		exit(1);
	}

	fcntl(comedi_fileno(dev),F_SETFL,O_NONBLOCK);

	map=mmap(NULL,0x1000,PROT_READ,MAP_SHARED,comedi_fileno(dev),0);
	printf("map=%p\n",map);

	do_cmd_1(dev);

	return 0;
}

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	unsigned int *chanlist;
	int n_chans;
	int total=0;
	int ret;
	int go;
	int i;

	chanlist = cmd->chanlist;
	n_chans = cmd->chanlist_len;

	ret=ioctl(comedi_fileno(dev),COMEDI_CMDTEST,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		perror("ioctl");
		return;
	}

	dump_cmd(cmd);

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;

	ret=ioctl(comedi_fileno(dev),COMEDI_CMDTEST,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("ioctl");
		return;
	}

	dump_cmd(cmd);

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;

	ret=ioctl(comedi_fileno(dev),COMEDI_CMD,cmd);

	printf("ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("ioctl");
		return;
	}

	go=1;
	while(go){
		ret=read(comedi_fileno(dev),buf,BUFSZ);
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
			total+=ret;
			printf("read %d %d\n",ret,total);
		}
	}

	for(i=0;i<100;i++){
		printf("%d\n",map[i]);
	}
}

/*
 * This part of the demo measures channels 1, 2, 3, 4 at a rate of
 * 10 khz, with the inter-sample time at 10 us (100 khz).  The number
 * of scans measured is 10.  This is analogous to the old mode2
 * acquisition.
 */
void do_cmd_1(comedi_t *dev)
{
	comedi_cmd cmd;
	unsigned int chanlist[4];

	memset(&cmd,0,sizeof(cmd));

	/* the subdevice that the command is sent to */
	cmd.subdev =	subdevice;

	/* flags */
	cmd.flags =	0;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

	/* In this case, we specify using TRIG_NOW to start
	 * acquisition immediately when the command is issued.  
	 * The argument of TRIG_NOW is "number of nsec after
	 * NOW", but no driver supports it yet.  Also, no driver
	 * currently supports using a start_src other than
	 * TRIG_NOW.  */
	cmd.start_src =		TRIG_NOW;
	cmd.start_arg =		0;

	/* The timing of the beginning of each scan is controlled
	 * by scan_begin.  TRIG_TIMER specifies that scan_start
	 * events occur periodically at a rate of scan_begin_arg
	 * nanoseconds between scans. */
	cmd.scan_begin_src =	TRIG_TIMER;
	cmd.scan_begin_arg =	100000;	/* in ns */

	/* The timing between each sample in a scan is controlled
	 * by convert.  Like above, TRIG_TIMER specifies that
	 * convert events occur periodically at a rate of convert_arg
	 * nanoseconds between scans. */
	cmd.convert_src =	TRIG_TIMER;
	cmd.convert_arg =	10000;		/* in ns */

	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */
	cmd.scan_end_src =	TRIG_COUNT;
	cmd.scan_end_arg =	4;		/* number of channels */

	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.  The src will typically be TRIG_COUNT or
	 * TRIG_NONE.  Specifying TRIG_COUNT will stop acquisition
	 * after stop_arg number of scans, or TRIG_NONE will
	 * cause acquisition to continue until stopped using
	 * comedi_cancel(). */
#if 1
	cmd.stop_src =		TRIG_COUNT;
	cmd.stop_arg =		100;
#else
	cmd.stop_src =		TRIG_NONE;
	cmd.stop_arg =		0;
#endif

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd.chanlist =		chanlist;
	cmd.chanlist_len =	4;

	chanlist[0]=CR_PACK(0,range,aref);
	chanlist[1]=CR_PACK(1,range,aref);
	chanlist[2]=CR_PACK(2,range,aref);
	chanlist[3]=CR_PACK(3,range,aref);

	do_cmd(dev,&cmd);
}

void do_cmd_2(comedi_t *dev)
{
	comedi_cmd cmd;
	unsigned int chanlist[4];

	memset(&cmd,0,sizeof(cmd));

	/* the subdevice that the command is sent to */
	cmd.subdev =	subdevice;

	/* flags */
	cmd.flags =	0;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

	cmd.start_src =		TRIG_NOW;
	cmd.start_arg =		0;

	cmd.scan_begin_src =	TRIG_TIMER;
	cmd.scan_begin_arg =	1;	/* in ns */

	cmd.convert_src =	TRIG_TIMER;
	cmd.convert_arg =	1;		/* in ns */

	cmd.scan_end_src =	TRIG_COUNT;
	cmd.scan_end_arg =	4;		/* number of channels */

#if 1
	cmd.stop_src =		TRIG_COUNT;
	cmd.stop_arg =		100;
#else
	cmd.stop_src =		TRIG_NONE;
	cmd.stop_arg =		0;
#endif

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd.chanlist =		chanlist;
	cmd.chanlist_len =	4;

	chanlist[0]=CR_PACK(0,range,aref);
	chanlist[1]=CR_PACK(1,range,aref);
	chanlist[2]=CR_PACK(2,range,aref);
	chanlist[3]=CR_PACK(3,range,aref);

	do_cmd(dev,&cmd);
}

char *cmd_src(int src,char *buf)
{
	buf[0]=0;

	if(src&TRIG_NONE)strcat(buf,"none|");
	if(src&TRIG_NOW)strcat(buf,"now|");
	if(src&TRIG_FOLLOW)strcat(buf, "follow|");
	if(src&TRIG_TIME)strcat(buf, "time|");
	if(src&TRIG_TIMER)strcat(buf, "timer|");
	if(src&TRIG_COUNT)strcat(buf, "count|");
	if(src&TRIG_EXT)strcat(buf, "ext|");
	if(src&TRIG_INT)strcat(buf, "int|");

	if(strlen(buf)==0){
		sprintf(buf,"unknown(0x%02x)",src);
	}else{
		buf[strlen(buf)-1]=0;
	}

	return buf;
}


void dump_cmd(comedi_cmd *cmd)
{
	char buf[100];

	printf("start: %s %d\n",
		cmd_src(cmd->start_src,buf),
		cmd->start_arg);

	printf("scan_begin: %s %d\n",
		cmd_src(cmd->scan_begin_src,buf),
		cmd->scan_begin_arg);

	printf("convert: %s %d\n",
		cmd_src(cmd->convert_src,buf),
		cmd->convert_arg);

	printf("scan_end: %s %d\n",
		cmd_src(cmd->scan_end_src,buf),
		cmd->scan_end_arg);

	printf("stop: %s %d\n",
		cmd_src(cmd->stop_src,buf),
		cmd->stop_arg);
}

