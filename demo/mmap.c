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
#include <unistd.h>
#include <sys/mman.h>
#include "examples.h"

int comedi_get_front_count(comedi_t *it, unsigned int subdev);

unsigned int chanlist[256];

void *map;

int prepare_cmd_lib(comedi_t *dev,int subdevice,comedi_cmd *cmd);
int prepare_cmd(comedi_t *dev,int subdevice,comedi_cmd *cmd);


int main(int argc, char *argv[])
{
	comedi_t *dev;
	comedi_cmd c,*cmd=&c;
	int size;
	int front, back;
	int ret;
	int i;

	parse_options(argc,argv);

	dev = comedi_open(filename);
	if(!dev){
		comedi_perror(filename);
		exit(1);
	}

	size = comedi_get_buffer_size(dev,subdevice);
	fprintf(stderr,"buffer size is %d\n",size);

	map=mmap(NULL,size,PROT_READ,MAP_SHARED,comedi_fileno(dev),0);
	fprintf(stderr,"map=%p\n",map);

	for(i=0;i<n_chan;i++){
		chanlist[i]=CR_PACK(channel+i,range,aref);
	}

	//prepare_cmd_lib(dev,subdevice,cmd);
	prepare_cmd(dev,subdevice,cmd);
	
	ret = comedi_command_test(dev,cmd);

	ret = comedi_command_test(dev,cmd);

	if(ret!=0){
		fprintf(stderr,"command_test failed\n");
		exit(1);
	}

	dump_cmd(stderr,cmd);

	ret = comedi_command(dev,cmd);
	if(ret<0){
		comedi_perror("comedi_command");
		exit(1);
	}

	back = 0;
	while(1){
		front = comedi_get_front_count(dev,subdevice);
		if(verbose)fprintf(stderr,"front = %d, back = %d\n",front,back);
		if(front<back)break;
		if(front==back){
			//comedi_poll(dev,subdevice);
			usleep(10000);
			continue;
		}

		for(i=back;i<front;i+=sizeof(sampl_t)){
			static int col = 0;
			printf("%d ",*(sampl_t *)(map+(i&(size-1))));
			col++;
			if(col==n_chan){
				printf("\n");
				col=0;
			}
		}

		ret = comedi_mark_buffer_read(dev,subdevice,front-back);
		if(ret<0){
			comedi_perror("comedi_mark_buffer_read");
			break;
		}
		back = front;
	}

	return 0;
}

int prepare_cmd_lib(comedi_t *dev,int subdevice,comedi_cmd *cmd)
{
	int ret;

	ret = comedi_get_cmd_generic_timed(dev,subdevice,cmd,1e9/freq);
	if(ret<0){
		comedi_perror("comedi_get_cmd_generic_timed\n");
		return ret;
	}

	cmd->chanlist = chanlist;
	cmd->chanlist_len = n_chan;
	cmd->scan_end_arg = n_chan;

	if(cmd->stop_src==TRIG_COUNT)cmd->stop_arg = n_scan;

	return 0;
}

int prepare_cmd(comedi_t *dev,int subdevice,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(*cmd));

	cmd->subdev = subdevice;

	cmd->flags = 0;

	cmd->start_src = TRIG_NOW;
	cmd->start_arg = 0;

	cmd->scan_begin_src = TRIG_TIMER;
	cmd->scan_begin_arg = 1e9/freq;

	cmd->convert_src = TRIG_TIMER;
	cmd->convert_arg = 1;

	cmd->scan_end_src = TRIG_COUNT;
	cmd->scan_end_arg = n_chan;

	cmd->stop_src = TRIG_COUNT;
	cmd->stop_arg = n_scan;

	cmd->chanlist = chanlist;
	cmd->chanlist_len = n_chan;

	return 0;
}



