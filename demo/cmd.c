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
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include "examples.h"

#define N_SCANS		10
#define N_CHANS		16

int n_chan = 4;
double freq = 1000;

#define BUFSZ 10000
char buf[BUFSZ];

unsigned int chanlist[N_CHANS];

void prepare_cmd_1(comedi_t *dev,comedi_cmd *cmd);
void prepare_cmd_2(comedi_t *dev,comedi_cmd *cmd);

void do_cmd(comedi_t *dev,comedi_cmd *cmd);
void dump_cmd(comedi_cmd *cmd);

int main(int argc, char *argv[])
{
	comedi_t *dev;
	comedi_cmd cmd;

	parse_options(argc,argv);

	dev = comedi_open(filename);
	if(!dev){
		perror(filename);
		exit(1);
	}

	fcntl(comedi_fileno(dev),F_SETFL,O_NONBLOCK);

	prepare_cmd_1(dev,&cmd);

	do_cmd(dev,&cmd);

	return 0;
}

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	unsigned int *chanlist;
	int n_chans;
	int total=0;
	int ret;
	int go;

	chanlist = cmd->chanlist;
	n_chans = cmd->chanlist_len;

	ret=comedi_command_test(dev,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(cmd);

#if 0
	/* restoring the chanlist stuff in this way is only required
	 * for comedi versions before 0.7.56
	 */ 
	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;
#endif

	ret=comedi_command_test(dev,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(cmd);

	ret=comedi_command(dev,cmd);

	printf("ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command");
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
			static int col = 0;
			int i;
			total+=ret;
			//printf("read %d %d\n",ret,total);
#if 1
			for(i=0;i<ret/2;i++){
				printf("%d ",((sampl_t *)buf)[i]);
				col++;
				if(col==n_chan){
					printf("\n");
					col=0;
				}
			}
#endif
		}
	}
}

/*
 * This part of the demo measures channels 1, 2, 3, 4 at a rate of
 * 10 khz, with the inter-sample time at 10 us (100 khz).  The number
 * of scans measured is 10.  This is analogous to the old mode2
 * acquisition.
 */
void prepare_cmd_1(comedi_t *dev,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(cmd));

	/* the subdevice that the command is sent to */
	cmd->subdev =	subdevice;

	/* flags */
	cmd->flags =	TRIG_WAKE_EOS;
	//cmd.flags =	0;

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
	cmd->start_src =		TRIG_NOW;
	cmd->start_arg =		0;

	/* The timing of the beginning of each scan is controlled
	 * by scan_begin.  TRIG_TIMER specifies that scan_start
	 * events occur periodically at a rate of scan_begin_arg
	 * nanoseconds between scans. */
	cmd->scan_begin_src =	TRIG_TIMER;
	cmd->scan_begin_arg =	10000000;

	/* The timing between each sample in a scan is controlled
	 * by convert.  Like above, TRIG_TIMER specifies that
	 * convert events occur periodically at a rate of convert_arg
	 * nanoseconds between scans. */
	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	10000;		/* in ns */

	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */
	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	n_chan;		/* number of channels */

	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.  The src will typically be TRIG_COUNT or
	 * TRIG_NONE.  Specifying TRIG_COUNT will stop acquisition
	 * after stop_arg number of scans, or TRIG_NONE will
	 * cause acquisition to continue until stopped using
	 * comedi_cancel(). */
	cmd->stop_src =		TRIG_COUNT;
	cmd->stop_arg =		10000;

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chan;

	chanlist[0]=CR_PACK(channel+0,range,aref);
	chanlist[1]=CR_PACK(channel+1,range,aref);
	chanlist[2]=CR_PACK(channel+2,range,aref);
	chanlist[3]=CR_PACK(channel+3,range,aref);
}

/*
 */
void prepare_cmd_2(comedi_t *dev,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(cmd));

	cmd->subdev =	subdevice;

	//cmd->flags =		TRIG_RT|TRIG_WAKE_EOS;
	cmd->flags =		0;

	cmd->start_src =	TRIG_NOW;
	cmd->start_arg =	0;

	cmd->scan_begin_src =	TRIG_TIMER;
	cmd->scan_begin_arg =	1;

	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	1000000;

	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	n_chan;

	cmd->stop_src =		TRIG_NONE;
	cmd->stop_arg =		0;

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chan;

	chanlist[0]=CR_PACK(channel+0,range,aref);
	chanlist[1]=CR_PACK(channel+1,range,aref);
	chanlist[2]=CR_PACK(channel+2,range,aref);
	chanlist[3]=CR_PACK(channel+3,range,aref);
}

