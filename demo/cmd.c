/*
 * Example of using commands - asynchronous input
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000,2001 David A. Schleef <ds@schleef.org>
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
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include "examples.h"

#define BUFSZ 10000
char buf[BUFSZ];

#define N_CHANS 256
unsigned int chanlist[N_CHANS];


int prepare_cmd_lib(comedi_t *dev,int subdevice,comedi_cmd *cmd);
int prepare_cmd(comedi_t *dev,int subdevice,comedi_cmd *cmd);

void do_cmd(comedi_t *dev,comedi_cmd *cmd);

char *cmdtest_messages[]={
	"success",
	"invalid source",
	"source conflict",
	"invalid argument",
	"argument conflict",
	"invalid chanlist",
};

int main(int argc, char *argv[])
{
	comedi_t *dev;
	comedi_cmd c,*cmd=&c;
	int ret;
	int total=0;
	int i;
	struct timeval start,end;

	parse_options(argc,argv);

	/* The following global variables used in this demo are
	 * defined in common.c, and can be modified by command line
	 * options.  When modifying this demo, you may want to
	 * change them here. */
	//filename = "/dev/comedi0";
	//subdevice = 0;
	//channel = 0;
	//range = 0;
	//aref = AREF_GROUND;
	//n_chan = 4;
	//n_scan = 1000;
	//freq = 1000.0;

	/* open the device */
	dev = comedi_open(filename);
	if(!dev){
		comedi_perror(filename);
		exit(1);
	}

	/* Set up channel list */
	for(i=0;i<n_chan;i++){
		chanlist[i]=CR_PACK(channel+i,range,aref);
	}

	/* prepare_cmd_lib() uses a Comedilib routine to find a
	 * good command for the device.  prepare_cmd() explicitly
	 * creates a command, which may not work for your device. */
	//prepare_cmd_lib(dev,subdevice,cmd);
	prepare_cmd(dev,subdevice,cmd);
	
	fprintf(stderr,"command before testing:\n");
	dump_cmd(stderr,cmd);

	/* comedi_command_test() tests a command to see if the
	 * trigger sources and arguments are valid for the subdevice.
	 * If a trigger source is invalid, it will be logically ANDed
	 * with valid values (trigger sources are actually bitmasks),
	 * which may or may not result in a valid trigger source.
	 * If an argument is invalid, it will be adjusted to the
	 * nearest valid value.  In this way, for many commands, you
	 * can test it multiple times until it passes.  Typically,
	 * if you can't get a valid command in two tests, the original
	 * command wasn't specified very well. */
	ret = comedi_command_test(dev,cmd);
	if(ret<0){
		comedi_perror("comedi_command_test");
		if(errno==EIO){
			fprintf(stderr,"Ummm... this subdevice doesn't support commands\n");
		}
		exit(1);
	}
	fprintf(stderr,"first test returned %d (%s)\n",ret,
			cmdtest_messages[ret]);
	dump_cmd(stderr,cmd);

	ret = comedi_command_test(dev,cmd);
	if(ret<0){
		comedi_perror("comedi_command_test");
		exit(1);
	}
	fprintf(stderr,"second test returned %d (%s)\n",ret,
			cmdtest_messages[ret]);
	if(ret!=0){
		dump_cmd(stderr,cmd);
		fprintf(stderr,"Error preparing command\n");
		exit(1);
	}

	/* start the command */
	ret=comedi_command(dev,cmd);
	if(ret<0){
		comedi_perror("comedi_command");
		exit(1);
	}

	/* this is only for informational purposes */
	gettimeofday(&start,NULL);
	fprintf(stderr,"start time: %ld.%06ld\n",start.tv_sec,start.tv_usec);

	while(1){
		ret=read(comedi_fileno(dev),buf,BUFSZ);
		if(ret<0){
			/* some error occurred */
			perror("read");
			break;
		}else if(ret==0){
			/* reached stop condition */
			break;
		}else{
			static int col = 0;
			total+=ret;
			if(verbose)fprintf(stderr,"read %d %d\n",ret,total);
			for(i=0;i<ret/2;i++){
				printf("%d ",((sampl_t *)buf)[i]);
				col++;
				if(col==n_chan){
					printf("\n");
					col=0;
				}
			}
		}
	}

	/* this is only for informational purposes */
	gettimeofday(&end,NULL);
	fprintf(stderr,"end time: %ld.%06ld\n",end.tv_sec,end.tv_usec);

	end.tv_sec-=start.tv_sec;
	if(end.tv_usec<start.tv_usec){
		end.tv_sec--;
		end.tv_usec+=1000000;
	}
	end.tv_usec-=start.tv_usec;
	fprintf(stderr,"time: %ld.%06ld\n",end.tv_sec,end.tv_usec);

	return 0;
}

/*
 * This prepares a command in a pretty generic way.  We ask the
 * library to create a stock command that supports periodic
 * sampling of data, then modify the parts we want. */
int prepare_cmd_lib(comedi_t *dev,int subdevice,comedi_cmd *cmd)
{
	int ret;

	/* This comedilib function will get us a generic timed
	 * command for a particular board.  If it returns -1,
	 * that's bad. */
	ret = comedi_get_cmd_generic_timed(dev,subdevice,cmd,1e9/freq);
	if(ret<0)return ret;

	/* Modify parts of the command */
	cmd->chanlist		= chanlist;
	cmd->chanlist_len	= n_chan;

	cmd->scan_end_arg = n_chan;
	if(cmd->stop_src==TRIG_COUNT)cmd->stop_arg = n_scan;

	return 0;
}

/*
 * Set up a command by hand.  This will not work on some devices.
 * There is no single command that will work on all devices.
 */
int prepare_cmd(comedi_t *dev,int subdevice,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(*cmd));

	/* the subdevice that the command is sent to */
	cmd->subdev =	subdevice;

	/* flags */
	cmd->flags = 0;

	/* Wake up at the end of every scan */
	//cmd->flags |= TRIG_WAKE_EOS;
	
	/* Use a real-time interrupt, if available */
	//cmd->flags |= TRIG_RT;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

	/* The start of acquisition is controlled by start_src.
	 * TRIG_NOW:     The start_src event occurs start_arg nanoseconds
	 *               after comedi_command() is called.  Currently,
	 *               only start_arg=0 is supported.
	 * TRIG_FOLLOW:  (For an output device.)  The start_src event occurs
	 *               when data is written to the buffer.
	 * TRIG_EXT:     start event occurs when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  start_arg chooses the particular digital
	 *               line.
	 * TRIG_INT:     start event occurs on a Comedi internal signal,
	 *               which is typically caused by an INSN_TRIG
	 *               instruction.
	 */
	cmd->start_src =	TRIG_NOW;
	cmd->start_arg =	0;

	/* The timing of the beginning of each scan is controlled by
	 * scan_begin.
	 * TRIG_TIMER:   scan_begin events occur periodically.
	 *               The time between scan_begin events is
	 *               convert_arg nanoseconds.
	 * TRIG_EXT:     scan_begin events occur when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  scan_begin_arg chooses the particular digital
	 *               line.
	 * TRIG_FOLLOW:  scan_begin events occur immediately after a scan_end
	 *               event occurs.
	 * The scan_begin_arg that we use here may not be supported exactly
	 * by the device, but it will be adjusted to the nearest supported
	 * value by comedi_command_test(). */
	cmd->scan_begin_src =	TRIG_TIMER;
	cmd->scan_begin_arg =	1e9/freq;		/* in ns */

	/* The timing between each sample in a scan is controlled by convert.
	 * TRIG_TIMER:   Conversion events occur periodically.
	 *               The time between convert events is
	 *               convert_arg nanoseconds.
	 * TRIG_EXT:     Conversion events occur when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  convert_arg chooses the particular digital
	 *               line.
	 * TRIG_NOW:     All conversion events in a scan occur simultaneously.
	 * Even though it is invalid, we specify 1 ns here.  It will be
	 * adjusted later to a valid value by comedi_command_test() */
	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	1;		/* in ns */

	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */
	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	n_chan;		/* number of channels */

	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.
	 * TRIG_COUNT:  stop acquisition after stop_arg scans.
	 * TRIG_NONE:   continuous acquisition, until stopped using
	 *              comedi_cancel()
	 * */
	cmd->stop_src =		TRIG_COUNT;
	cmd->stop_arg =		n_scan;

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chan;

	return 0;
}


