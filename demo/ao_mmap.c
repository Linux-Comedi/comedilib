/*
 * Asynchronous Analog Output Example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2005 Frank Mori Hess <fmhess@@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * Requirements: Analog output device capable of
 *    asynchronous commands.
 *
 * This demo uses an analog output subdevice with an
 * asynchronous command to generate a waveform.  The
 * waveform in this example is a sine wave (surprise!).
 * The waveform data is passed to comedi through
 * a memory mapping (as opposed to using write()).
 * The entire buffer is filled once with one period
 * of the waveform.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include "examples.h"


static int comedi_internal_trigger(comedi_t *dev, unsigned int subd, unsigned int trignum)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_INTTRIG;
	insn.subdev = subd;
	insn.data = data;
	insn.n = 1;

	data[0] = trignum;

	return comedi_do_insn(dev, &insn);
}

static void write_waveform(sampl_t *buffer, int size, double amplitude, double offset, int maxdata)
{
	int i;
	
	for(i = 0; i < size; ++i)
	{
		double temp = (amplitude / 2.) * sin((2. * M_PI * i) / size) + offset;
		if(temp < 0.) temp = 0.;
		if(temp > maxdata) temp = maxdata;
		buffer[i] = (sampl_t)temp;
	}
}

int main(int argc, char *argv[])
{
	comedi_cmd cmd;
	int err;
	comedi_t *dev;
	unsigned int chanlist[16];
	unsigned int maxdata;
	comedi_range *rng;
	int ret;
	int size;
	sampl_t *map;
	/* peak-to-peak amplitude, in DAC units (i.e., 0-4095) */
	double amplitude;
	/* offset, in DAC units */
	double offset;
	int subdevice;

	
	parse_options(argc,argv);

	/* Force n_chan to be 1 */
	n_chan = 1;

	dev = comedi_open(filename);
	if(dev == NULL){
		fprintf(stderr, "error opening %s\n", filename);
		return -1;
	}
	subdevice = comedi_find_subdevice_by_type(dev,COMEDI_SUBD_AO,0);

	maxdata = comedi_get_maxdata(dev,subdevice,0);
	rng = comedi_get_range(dev,subdevice,0,0);

	offset = (double)comedi_from_phys(0.0, rng, maxdata);
	amplitude = (double)comedi_from_phys(1.0, rng, maxdata) - offset;

	memset(&cmd,0,sizeof(cmd));
	cmd.subdev = subdevice;
	cmd.flags = 0;
	cmd.start_src = TRIG_INT;
	cmd.start_arg = 0;
	cmd.scan_begin_src = TRIG_TIMER;
	cmd.scan_begin_arg = 1e9/freq;
	cmd.convert_src = TRIG_NOW;
	cmd.convert_arg = 0;
	cmd.scan_end_src = TRIG_COUNT;
	cmd.scan_end_arg = n_chan;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;

	cmd.chanlist = chanlist;
	cmd.chanlist_len = n_chan;

	chanlist[0] = CR_PACK(channel,range,aref);

	dump_cmd(stdout,&cmd);

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	if ((err = comedi_command(dev, &cmd)) < 0) {
		comedi_perror("comedi_command");
		exit(1);
	}
	
	size = comedi_get_buffer_size(dev, subdevice);
	fprintf(stderr, "buffer size is %d\n", size);
	map = mmap(NULL, size, PROT_WRITE, MAP_SHARED, comedi_fileno(dev), 0);
	if(map == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	write_waveform(map, size / sizeof(sampl_t), amplitude, offset, maxdata);
	if(msync(map, size, MS_SYNC) < 0)
	{
		perror("msync");
		exit(1);
	}
	if(comedi_mark_buffer_written(dev, subdevice, size) < 0)
	{
		comedi_perror("comedi_mark_buffer_written");
		exit(1);
	}
	ret = comedi_internal_trigger(dev, subdevice, 0);
	if(ret<0){
		comedi_perror("comedi_internal_trigger\n");
		exit(1);
	}
	while(1){
		int bytes_marked = comedi_get_buffer_contents(dev,subdevice);
		if(bytes_marked < 1)
		{
			comedi_perror("comedi_get_buffer_contents");
			exit(1);
		}
		int bytes_unmarked = size - bytes_marked;
		// this keeps comedi from reporting a buffer underrun
		if(comedi_mark_buffer_written(dev, subdevice, bytes_unmarked) < 0)
		{
			comedi_perror("comedi_mark_buffer_written");
			exit(1);
		}
	}
	return 0;
}

