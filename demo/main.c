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


	while (optind<argc) {
		c = getopt(argc, argv, "acsrfvdgom");
		switch (c) {
		case -1:
			/* data value */
			sscanf(argv[optind],"%d",&value);
			optind++;
			break;
		case 'f':
			filename = argv[optind];
			optind++;
			break;
		case 's':
			sscanf(argv[optind],"%d",&subdevice);
			optind++;
			break;
		case 'c':
			sscanf(argv[optind],"%d",&channel);
			optind++;
			break;
		case 'a':
			sscanf(argv[optind],"%d",&aref);
			optind++;
			break;
		case 'r':
			sscanf(argv[optind],"%d",&range);
			optind++;
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



