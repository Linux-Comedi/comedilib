#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
/*
 * A little pwm demo
 * Part of Comedilib
 *
 * Copyright (c) 2012 Bernd Porr <berndporr@f2s.com>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include "examples.h"

// the option -n is used to switch on/off the pwm

int main(int argc, char *argv[])
{
        int ret,i;
	comedi_insn insn;
	lsampl_t d[5];
	comedi_t *device;

        int freq;

	struct parsed_options options;

	init_parsed_options(&options);
	options.freq = -1;
	// we hijack this option to switch it on or off
	options.n_scan = -1;
	options.value = -1;
	parse_options(&options, argc, argv);

	if ((options.value==-1)&&(options.n_scan==-1)&&(options.freq==-1)) {
		fprintf(stderr,
			"Usage: %s OPTIONS duty_cycle\n"
			"options: \n"
			"     -N 0    switches PWM off\n"
			"     -N 1    switches PWM on\n"
                        "     -N 2    enquires the max value for the duty cycle\n"
			"     -F FREQ sets the PWM frequency\n",
			argv[0]);
	}

        device = comedi_open(options.filename);
        if(!device){
		comedi_perror(options.filename);
		exit(-1);
        }

	options.subdevice = comedi_find_subdevice_by_type(device,COMEDI_SUBD_PWM,0);
	if (options.verbose)
		printf("PWM subdevice autodetection gave subdevice number %d\n",
		       options.subdevice);

	if(options.n_scan==2) {
		printf("%d\n",comedi_get_maxdata(device,options.subdevice,0));
		close(device);
		exit(0);
	}       

	insn.insn=INSN_CONFIG;
	insn.data=d;
	insn.subdev=options.subdevice;
	insn.chanspec=CR_PACK(0,0,0);

	if(options.n_scan==1) {
		d[0] = INSN_CONFIG_ARM;
		d[1] = 0;
		insn.n=2;
		ret=comedi_do_insn(device,&insn);
		if(ret < 0){
			fprintf(stderr,"Could not switch on:%d\n",ret);
			comedi_perror(options.filename);
			exit(-1);
		}
	}
	if(options.n_scan==0) {
		d[0] = INSN_CONFIG_DISARM;
		d[1] = 0;
		insn.n=1;
		ret=comedi_do_insn(device,&insn);
		if(ret < 0){
			fprintf(stderr,"Could not switch off:%d\n",ret);
			comedi_perror(options.filename);
			exit(-1);
		}
	}
	if(options.freq>0) {
		freq = options.freq;
		d[0] = INSN_CONFIG_PWM_SET_PERIOD;
		d[1] = 1E9/freq;
		insn.n=2;
		ret=comedi_do_insn(device,&insn);
		if(ret < 0){
			fprintf(stderr,"Could set frequ:%d\n",ret);
			comedi_perror(options.filename);
			exit(-1);
		}
	}
		
	d[0] = INSN_CONFIG_GET_PWM_STATUS;
	insn.n=2;
	ret=comedi_do_insn(device,&insn);
	if(ret < 0){
		fprintf(stderr,"Could not get status:%d insn=%d\n",
			ret,
			d[0]);
		comedi_perror(options.filename);
		exit(-1);
	}
	if (options.verbose) {
		if (d[1])
			fprintf(stderr,
				"PWM is on.\n");
		else
			fprintf(stderr,
				"PWM is off.\n");
	}
	d[0] = INSN_CONFIG_PWM_GET_PERIOD;
	insn.n=2;
	ret=comedi_do_insn(device,&insn);
	if(ret < 0){
		fprintf(stderr,"Could get frequ:%d\n",ret);
		comedi_perror(options.filename);
		exit(-1);
	}
	freq = 1E9 / d[1];
	if (options.verbose)
		fprintf(stderr,"PWM frequency is %d\n", freq);

	if (options.value>=0)

		if(comedi_data_write(device,
				     options.subdevice, 
				     options.channel,
				     0,
				     0,
				     options.value)<0)
		{
			fprintf(stderr,"error setting the pwm duty cycle on ");
			comedi_perror(options.filename);
			exit(1);
		}

        return 0;
}
