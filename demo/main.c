/*
 * This is a little helper function to parse options that
 * are common to most of the examples.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include "examples.h"


char *filename="/dev/comedi0";
int verbose_flag;

int value=0;
int subdevice=0;
int channel=0;
int aref=0;
int range=0;


int parse_options(int argc, char *argv[])
{
	int c;


	while (-1 != (c = getopt(argc, argv, "a:c:s:r:f:vdgom"))) {
		switch (c) {
		case 'f':
			filename = optarg;
			break;
		case 's':
			sscanf(optarg,"%d",&subdevice);
			break;
		case 'c':
			sscanf(optarg,"%d",&channel);
			break;
		case 'a':
			sscanf(optarg,"%d",&aref);
			break;
		case 'r':
			sscanf(optarg,"%d",&range);
			break;
		case 'v':
			verbose_flag = 1;
			break;
		case 'd':
			aref=AREF_DIFF;
			break;
		case 'g':
			aref=AREF_GROUND;
			break;
		case 'o':
			aref=AREF_OTHER;
			break;
		case 'm':
			aref=AREF_COMMON;
			break;
		default:
			printf("bad option\n");
			exit(1);
		}
	}
	if(optind < argc) {
		/* data value */
		sscanf(argv[optind++],"%d",&value);
	}

	return argc;
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

