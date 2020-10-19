/*
 * ext_trig demo
 * Part of Comedilib
 *
 * Copyright (c) 2001 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2020 Bernd Harries <bharries@web.de>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * Requirements:
 *    - A board with a subdevice that
 *      can trigger on an external digital line.
 *      A parallel port satisfies these requirements.
 *      Adlink PCI 7230 with adequate driver, too.
 *      Advantech PCI 1730  with adequate driver, too.
 *
 * The program sets up a comedi command to wait for an external trigger.
 * In a loop, waits for the input subdevice in select() with
 * a long timeout. If select returns with the file descrptor
 * ready for read, the available input data is read from the
 * file descrptor and getimeofday() is called. The absolute
 * and the delta time since te last trigger event is printed as
 * well as the first two 16 bit samples possibly in the read-buffer.
 *
 * 2 instances per card can run at the same time if you have
 * Adlink PCI 7230 card(s) installed.
 * 4 instances per card can run at the same time if you have
 * Advantech PCI 1730 card(s) installed.
 * 1 instances per parport can run at the same time if you have
 * parallel ports installed.
 *
 * 5 parallel instances have successfully been used in a constellation
 * with 1 PCIe parport, 1 LPCIe 7230 and 1 PCI 1730 card.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "examples.h"


comedi_t *device;

int count;

int out_subd;

#define BUFSZ 1024
sampl_t buf[BUFSZ];

unsigned int chanlist[16];


void prepare_cmd(comedi_t *dev,comedi_cmd *cmd, int subdevice);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);


int main(int argc, char *argv[])
{
	int ret;
	comedi_cmd cmd;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	fprintf(stderr, "\n now comedi_open(%s ) ... ", options.filename);
	device = comedi_open(options.filename);
	if(!device){
		perror(options.filename);
		exit(1);
	}

	ret = fcntl(comedi_fileno(device),F_SETFL,O_NONBLOCK|O_ASYNC);
	if(ret<0)perror("fcntl");

#if 0
	{
	struct sched_param p;

	memset(&p,0,sizeof(p));
	p.sched_priority = 1;
	ret = sched_setscheduler(0,SCHED_FIFO,&p);
	if(ret<0)perror("sched_setscheduler");
	}
#endif

	fprintf(stderr, "\n now prepare_cmd(sd = %d ) ... ", options.subdevice);
	prepare_cmd(device, &cmd, options.subdevice);

	fprintf(stderr, "\n now do_cmd(dev, &cmd ) ... ");
	do_cmd(device, &cmd);

	return 0;
}

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	int total = 0;
	int ret;
	int go;
	fd_set rdset;
	struct timeval timeout;
	struct timeval tvs_last = {0, 0};
	struct timeval tvs_now = {0, 0};;
	struct timeval tvs_delta = {0, 0};;

	ret = comedi_command_test(dev, cmd);

	fprintf(stderr, "\ncmd-test ret=%d ", ret);
	if(ret < 0){
		fprintf(stderr, "\nerrno=%d", errno);
		comedi_perror("comedi_command_test()");
		return;
	}

	dump_cmd(stderr, cmd);

	ret = comedi_command_test(dev, cmd);

	fprintf(stderr, "\ncmd-test ret=%d ", ret);
	if(ret < 0){
		fprintf(stderr, "\nerrno=%d", errno);
		comedi_perror("comedi_command_test()");
		return;
	}

	dump_cmd(stderr, cmd);

	comedi_set_read_subdevice(dev, cmd->subdev);
	ret = comedi_get_read_subdevice(dev);
	if (ret < 0 || ret != cmd->subdev) {
		fprintf(stderr,
			"failed to change 'read' subdevice from %d to %d",
			ret, cmd->subdev);
		return;
	}

	fprintf(stderr, "\n now comedi_command(dev, &cmd ) ... ");
	ret=comedi_command(dev, cmd);

	fprintf(stderr, "\nret=%d", ret);
	if(ret < 0)
	{
		fprintf(stderr, "\nerrno=%d", errno);
		comedi_perror("comedi_command");
		return;
	}

	ret = gettimeofday(&tvs_last, NULL);
	go = 1;
	while(go)
	{
		FD_ZERO(&rdset);
		FD_SET(comedi_fileno(dev), &rdset);
		timeout.tv_sec = 16;
		timeout.tv_usec = 50000;
		fprintf(stderr, "\n now select(comedi_fileno(dev) + 1, &rdset, NULL, NULL, &(%02ld.%06ld)) ... ",
		timeout.tv_sec, timeout.tv_usec);
		ret = select(comedi_fileno(dev) + 1, &rdset, NULL, NULL, &timeout);
		if(ret < 0)
		{
			perror("select");
		}else if(ret == 0)
		{
			/* timeout */
			fprintf(stderr, "TiO ");
		}
		else if(FD_ISSET(comedi_fileno(dev), &rdset))
		{
			ret=read(comedi_fileno(dev), buf, BUFSZ);
			if(ret < 0)
			{
				if(errno == EAGAIN){
					go = 0;
					perror("read");
				}
			}else if(ret == 0)  // no Data from read()
			{
				go = 0;
			}else{
				//int i;

				//fprintf(stderr, "\n now gettimeofday() ... ");
				ret = gettimeofday(&tvs_now, NULL);

				total += ret;
				//fprintf(stderr, "\nread %d %d", ret, total);
				//fprintf(stderr, "\ncount = %d", count);

				tvs_delta.tv_usec = tvs_now.tv_usec - tvs_last.tv_usec;
				tvs_delta.tv_sec = tvs_now.tv_sec - tvs_last.tv_sec;
				if(tvs_delta.tv_usec < 0)
				{
					tvs_delta.tv_usec += 1000000;
					tvs_delta.tv_sec -= 1;
				}
				//endif(tvs_delta.tv_usec < 0)
				tvs_last = tvs_now;
				
				fprintf(stderr, "\n t=%12ld.%06ld s dt=%4ld.%06ld s  buf=%04hX %04hX ",
					tvs_now.tv_sec, tvs_now.tv_usec,
					tvs_delta.tv_sec, tvs_delta.tv_usec,
					buf[0], buf[1]);

			}
			//endif(read_ret == 0)
		}
	}
}
//endproc do_cmd()

/*
 * prepare a comedi command to configure a subdevice for
 * external triggering.
 */
void prepare_cmd(comedi_t *dev, comedi_cmd *cmd, int subdevice)
{
	memset(cmd, 0, sizeof(*cmd));

	/* the subdevice that the command is sent to */
	cmd->subdev = subdevice;

	/* flags */
	cmd->flags =		TRIG_WAKE_EOS;

	cmd->start_src =	TRIG_NOW;
	cmd->start_arg =	0;

	cmd->scan_begin_src =	TRIG_EXT;
	cmd->scan_begin_arg =	0;

#if 0
	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	1;
#else
	cmd->convert_src =	TRIG_ANY;
	cmd->convert_arg =	0;
#endif

	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	1;

	cmd->stop_src =		TRIG_NONE;
	cmd->stop_arg =		0;

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	1;

	chanlist[0]=CR_PACK(0, 0, 0);
}
//endproc prepare_cmd()
