/*
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <malloc.h>


char *filename="/dev/comedi0";
int verbose_flag;
comedi_t *device;

int value;
int subdevice;
int channel;
int aref;
int range;


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



