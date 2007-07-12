/*
 * INSN_CONFIG_FILTER example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 * Copyright (c) 2007 Frank Mori Hess <fmhess@users.sourceforge.net>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a subdevice that supports
 *    INSN_CONFIG_FILTER, such as the PFI subdevice on an NI m-series
 *    or 660x board.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "examples.h"


comedi_t *device;

int main(int argc, char *argv[])
{
	int retval;
	lsampl_t filter_selection;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}
	filter_selection = options.value;
	printf("Selecting filter %d on subdevice %d channel %d.\n", filter_selection, options.subdevice, options.channel);
	comedi_insn insn;
	lsampl_t data[2];
	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = options.subdevice;
	insn.chanspec = options.channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_FILTER;
	data[1] = filter_selection;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0) comedi_perror("comedi_do_insn");
	return retval;
}

