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

int subdevice;
int channel;
int aref;
int range;


int parse_options(int argc, char *argv[])
{
	int c;


	while (1) {
		c = getopt(argc, argv, "acsrfvdgom");
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			filename = argv[optind];
			break;
		case 's':
			sscanf(argv[optind],"%d",&subdevice);
			break;
		case 'c':
			sscanf(argv[optind],"%d",&channel);
			break;
		case 'a':
			sscanf(argv[optind],"%d",&aref);
			break;
		case 'r':
			sscanf(argv[optind],"%d",&range);
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

	return argc;
}



