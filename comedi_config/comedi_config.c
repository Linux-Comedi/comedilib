/*
    comedi_config/comedi_config.c
    configuration program for comedi kernel module

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998,2000 David A. Schleef <ds@stm.lbl.gov>

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

#define CC_VERSION	"0.7.13"

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

int quiet=0,verbose=0;

int read_buf_flag=0;
int read_buf_size=0;
int write_buf_flag=0;
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
	{ "read-buffer", 1, &read_buf_flag, 1},
	{ "write-buffer", 1, &write_buf_flag, 1},
};

void do_help(int i)
{
	fputs(
		"comedi_config version " CC_VERSION "\n"
		"usage:  comedi_config [OPTIONS] <device file> [<driver> <opt1>,<opt2>,...]\n"
		"\n"
		"OPTIONS:\n"
		"\t-v --verbose\n"
			"\t\tverbose output\n"
		"\t-q --quiet\n"
			"\t\tquiet output\n"
		"\t-V --version\n"
			"\t\tprint program version\n"
		"\t-i --init-data <filename>\n"
			"\t\tI don't know what this does\n"
		"\t--read-buffer <size>\n"
			"\t\tset buffer size in kilobytes used for reads from the <device file>\n"
		"\t--write-buffer <size>\n"
			"\t\tset buffer size in kilobytes used for writes to the <device file>\n"
		"\n"
		"<optN> are integers whose interpretation depends on\n"
		"the driver.  In general, however, for non-plug-and-play boards\n"
		"opt1 refers to the I/O port address and opt2 refers to IRQ number\n"
		"to be used by the driver.  For plug-and-play boards, opt1 and opt2\n"
		"are optional and allow you to specify the bus and slot of the card\n"
		"(in case you have two identical cards).\n"
		,stderr);
	exit(i);
}

int main(int argc,char *argv[])
{
	comedi_devconfig it;
	comedi_bufconfig bc;
	int fd;
	int c,i,num,k;
	char *opts;
	char *fn,*driver;
	struct stat statbuf;
	int ret;
	int remove=0;
	int index;

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
		case 0:
			if(read_buf_flag) read_buf_size = strtol(optarg, NULL, 0);
			if(write_buf_flag) write_buf_size = strtol(optarg, NULL, 0);
			if(read_buf_size < 0 || write_buf_size < 0)
			{
				fprintf(stderr, "invalid buffer size\n");
				exit(-1);
			}
			break;
		default:
			do_help(1);
		}
	}

	if((argc-optind) < 1 || (argc-optind) > 3 ||
		((argc-optind) == 1 && read_buf_flag == 0 && write_buf_flag == 0)){
		do_help(1);
	}

	fn=argv[optind];

	fd=open(fn,O_RDWR);
	if(fd<0){
		perror(fn);
		exit(1);
	}

	// if we are attaching a device and not just changing buffer size
	if((argc-optind) > 1)
	{
		driver=argv[optind+1];
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
				perror("allocating initialization data\n");
				exit(1);
			}

			ret = read(init_fd,init_data,init_size);
			if(ret<0){
				perror("reading initialization data\n");
				exit(1);
			}

			it.options[0]=(int)init_data;
			it.options[1]=init_size;
		}

	/* add: sanity check for device */

		if(verbose){
			printf("configuring driver=%s ",it.board_name);
			for(i=0;i<COMEDI_NDEVCONFOPTS;i++)printf("%d,",it.options[i]);
			printf("\n");
		}
		if(ioctl(fd,COMEDI_DEVCONFIG,remove?NULL:&it)<0){
			int err=errno;
			perror("Configure failed!");
			fprintf(stderr,"Check kernel log for more information\n");
			fprintf(stderr,"Possible reasons for failure:\n");
			switch(err){
			case EINVAL:
				fprintf(stderr,"  \n");
				break;
			case EBUSY:
				fprintf(stderr,"  Already configured\n");
				break;
			case EIO:
				fprintf(stderr,"  Driver not found\n");
				break;
			case EPERM:
				fprintf(stderr,"  Not root\n");
				break;
			case EFAULT:
				fprintf(stderr,"  Comedi bug\n");
				break;
			default:
				fprintf(stderr,"  Unknown\n");
				break;
			}

			exit(1);
		}
	}

	// do buffer resizing
	if(read_buf_flag || write_buf_flag)
	{
		memset(&bc, 0, sizeof(bc));
		bc.read_size = read_buf_size * 1024;
		bc.write_size = write_buf_size * 1024;
		if(ioctl(fd, COMEDI_BUFCONFIG, &bc) < 0)
		{
			perror("buffer resize error");
		}
		if(verbose)
		{
			if(read_buf_size) printf("%s read buffer resized to %i kilobytes\n",
				fn, bc.read_size / 1024);
			if(write_buf_size) printf("%s write buffer resized to %i kilobytes\n",
				fn, bc.write_size / 1024);
		}
	}

	exit(0);
}

