/*
    comedi_config/comedi_config.c
    configuration program for comedi kernel module

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998,2000 David A. Schleef <ds@schleef.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#define CC_VERSION	"0.7.16"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <comedi.h>

#ifdef I18N
#include <libintl.h>
#define _(a) gettext(a)
#else
#define _(a) (a)
#endif

int quiet=0,verbose=0;

int read_buf_size=0;
int write_buf_size=0;

int init_fd;
char *init_file;
void *init_data;
int init_size;

struct option options[] = {
	{ "verbose", 0, 0, 'v' },
	{ "quiet", 0, 0, 'q' },
	{ "version", 0, 0, 'V' },
	{ "init-data", 1, 0, 'i' },
	{ "remove", 0, 0, 'r' },
	{ "read-buffer", 1, NULL, 0x1000},
	{ "write-buffer", 1, NULL, 0x1001},
	{ 0 },
};

void do_help(int i)
{
	fputs("comedi_config version " CC_VERSION "\n",stderr);
	fputs(
_("usage:  comedi_config [OPTIONS] <device file> [<driver> <opt1>,<opt2>,...]\n"
"\n"
"OPTIONS:\n"
"  -v, --verbose\n"
"      verbose output\n"
"  -q, --quiet\n"
"      quiet output\n"
"  -V, --version\n"
"      print program version\n"
"  -i, --init-data <filename>\n"
"      Use file for driver initialization data, typically firmware code.\n"
"  -r, --remove\n"
"      remove previously configured driver\n"
"  --read-buffer <size>\n"
"      set buffer size in kilobytes used for reading\n"
"  --write-buffer <size>\n"
"      set buffer size in kilobytes used for writing\n"
"\n"
"  <optN> are integers whose interpretation is driver dependent.\n"
"  In general, for PCI boards, <opt1> and <opt2> refer to the bus/slot\n"
"  indices of the board.  If not specified, a board will automatically\n"
"  be chosen.  For non-PCI boards, <opt1> specifies the I/O port base\n"
"  address and, if applicable, <opt2> specifies the IRQ.  Additional\n"
"  options may be useful, see the Comedi documentation for details.\n")
		,stderr);
	exit(i);
}

int main(int argc,char *argv[])
{
	comedi_devconfig it;
	comedi_bufconfig bc;
	comedi_devinfo devinfo;
	int fd;
	int c,i,num,k;
	char *opts;
	char *fn,*driver;
	struct stat statbuf;
	int ret;
	int remove=0;
	int index;

#ifdef I18N
	setlocale(LC_ALL, "");
	bindtextdomain("comedilib", "/usr/share/locale");
	textdomain("comedilib");
#endif

	if(geteuid() != 0)
		fprintf(stderr,_("comedi_config should be run as root.  Attempting to continue anyway.\n"));

	while(1){
		c=getopt_long(argc, argv, "rvVqi:", options, &index);
		if(c==-1)break;
		switch(c){
		case 'v':
			verbose=1;
			break;
		case 'V':
			fputs("comedi_config version " CC_VERSION "\n",stderr);
			exit(0);
			break;
		case 'q':
			quiet=1;
			break;
		case 'r':
			remove=1;
			break;
		case 'i':
			init_file=optarg;
			break;
		case 0x1000:
			read_buf_size = strtol(optarg, NULL, 0);
			if(read_buf_size < 0)
			{
				fprintf(stderr, _("invalid buffer size\n"));
				exit(-1);
			}
			break;
		case 0x1001:
			write_buf_size = strtol(optarg, NULL, 0);
			if(write_buf_size < 0)
			{
				fprintf(stderr, _("invalid buffer size\n"));
				exit(-1);
			}
			break;
		default:
			do_help(1);
		}
	}

	if((argc-optind) < 1 || (argc-optind) > 3 ||
		((argc-optind) == 1 && read_buf_size == 0 && write_buf_size == 0 && remove == 0)){
		do_help(1);
	}

	fn=argv[optind];

	fd=open(fn,O_RDWR);
	if(fd<0){
		switch(errno){
		case ENODEV:
			fprintf(stderr,_("comedi.o not loaded\n"));
			break;
		case ENXIO:
			fprintf(stderr,_("device not configured\n"));
			break;
		case EPERM:
			fprintf(stderr,_("modprobe problem\n"));
			break;
		default:
			perror(fn);
			break;
		}
		exit(1);
	}

	// if we are attaching or detaching a device
	if((argc-optind) > 1 || ((argc-optind) == 1 && remove))
	{
		if(argc - optind > 1)
			driver=argv[optind+1];
		else
			driver = "none";
		strncpy(it.board_name,driver,COMEDI_NAMELEN-1);

		for(i=0;i<COMEDI_NDEVCONFOPTS;i++)it.options[i]=0;

		if((argc-optind)==3){
			opts=argv[optind+2];
			i=0;
			while(*opts){
				if((*opts)==','){
					i++;
					opts++;
					if(i>=COMEDI_NDEVCONFOPTS)
						do_help(1);
					continue;
				}
				if(sscanf(opts,"%i%n",&num,&k)>0){
					it.options[i]=num;
					opts+=k;
					continue;
				}
				do_help(1);
			}
		}

		ret=stat(fn,&statbuf);
		if(ret<0){
			perror(fn);
			exit(1);
		}
#if 0
		/* this appears to be broken */
		if(  !(S_ISCHR(statbuf.st_mode)) ||
		     major(statbuf.st_dev)!=COMEDI_MAJOR){
			if(!quiet)
				fprintf(stderr,"warning: %s might not be a comedi device\n",fn);
		}
#endif

		if(init_file){
			struct stat buf;

			init_fd = open(init_file,O_RDONLY);
			if(init_fd<0){
				perror(init_file);
				exit(1);
			}

			fstat(init_fd,&buf);

			init_size = buf.st_size;
			init_data = malloc(init_size);
			if(init_data==NULL){
				perror(_("allocating initialization data\n"));
				exit(1);
			}

			ret = read(init_fd,init_data,init_size);
			if(ret<0){
				perror(_("reading initialization data\n"));
				exit(1);
			}

			it.options[COMEDI_DEVCONF_AUX_DATA]=(int)init_data;
			it.options[COMEDI_DEVCONF_AUX_DATA_LENGTH]=init_size;
		}

	/* add: sanity check for device */

		if(verbose){
			printf(_("configuring driver=%s "),it.board_name);
			for(i=0;i<COMEDI_NDEVCONFOPTS;i++)printf("%d,",it.options[i]);
			printf("\n");
		}
		if(ioctl(fd,COMEDI_DEVCONFIG,remove?NULL:&it)<0){
			int err=errno;
			perror(_("Configure failed!"));
			fprintf(stderr,_("Check kernel log for more information\n"));
			fprintf(stderr,_("Possible reasons for failure:\n"));
			switch(err){
			case EINVAL:
				fprintf(stderr,"  \n");
				break;
			case EBUSY:
				fprintf(stderr,_("  Already configured\n"));
				break;
			case EIO:
				fprintf(stderr,_("  Driver not found\n"));
				break;
			case EPERM:
				fprintf(stderr,_("  Not root\n"));
				break;
			case EFAULT:
				fprintf(stderr,_("  Comedi bug\n"));
				break;
			default:
				fprintf(stderr,_("  Unknown\n"));
				break;
			}

			exit(1);
		}
	}

	// dont do buffer resize if we have removed device
	if(remove == 0)
	{
		if(read_buf_size || write_buf_size){
			ret = ioctl(fd,COMEDI_DEVINFO,&devinfo);
			if(ret<0){
				perror("devinfo");
				exit(1);
			}
			if(devinfo.version_code < ((7<<8) | (57))){
				fprintf(stderr,_("Buffer resizing requires Comedi version >= 0.7.57\n"));
				exit(1);
			}
		}

		// do buffer resizing
		if(read_buf_size)
		{
			if(devinfo.read_subdevice < 0){
				fprintf(stderr,_("warning: no read subdevice, resize ignored\n"));
			}else{
				memset(&bc, 0, sizeof(bc));
				bc.subdevice = devinfo.read_subdevice;
				bc.maximum_size = read_buf_size * 1024;
				bc.size = read_buf_size * 1024;
				if(ioctl(fd, COMEDI_BUFCONFIG, &bc) < 0)
				{
					perror(_("buffer resize error"));
					exit(1);
				}
				if(verbose)
				{
					printf(_("%s read buffer resized to %i kilobytes\n"),
						fn, bc.size / 1024);
				}
			}
		}
		if(write_buf_size)
		{
			if(devinfo.write_subdevice < 0){
				fprintf(stderr,_("warning: no write subdevice, resize ignored\n"));
			}else{
				memset(&bc, 0, sizeof(bc));
				bc.subdevice = devinfo.write_subdevice;
				bc.maximum_size = write_buf_size * 1024;
				bc.size = write_buf_size * 1024;
				if(ioctl(fd, COMEDI_BUFCONFIG, &bc) < 0)
				{
					perror(_("buffer resize error"));
					exit(1);
				}
				if(verbose)
				{
					printf(_("%s write buffer resized to %i kilobytes\n"),
						fn, bc.size / 1024);
				}
			}
		}
	}

	exit(0);
}

