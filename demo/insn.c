/*
   A little input demo
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

extern int verbose_flag;
extern int subdevice;
extern int range;
extern int channel;
extern int aref;
extern char *filename;

comedi_t *device;


int main(int argc, char *argv[])
{
	int ret;
	comedi_insn insn;
	comedi_insnlist il;
	lsampl_t data;

	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	if(verbose_flag){
		printf("measuring device=%s subdevice=%d channel=%d range=%d analog reference=%d\n",
			filename,subdevice,channel,range,aref);
	}

	il.n_insns=1;
	il.insns=&insn;

	insn.insn=INSN_READ;
	insn.n=1;
	insn.data=&data;
	insn.subdev=subdevice;
	insn.chanspec=CR_PACK(channel,range,aref);

	ret=ioctl(comedi_fileno(device),COMEDI_INSNLIST,&il);
	//ret=comedi_data_read(device,subdevice,channel,range,aref,&data);
	if(ret<0){
		comedi_perror(filename);
		exit(0);
	}

	printf("%d\n",data);

	return 0;
}

