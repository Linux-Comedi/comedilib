/*
   A little input demo for commands

 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>

#define N_SCANS		10
#define N_CHANS		16

int subdevice = 0;
int chan=0;
int range = 0;
int aref = AREF_GROUND;
double freq = 1000;

#define BUFSZ 100
char buf[BUFSZ];


static void do_cmd(comedi_t *dev);

int main(int argc, char *argv[])
{
	char *fn = NULL;
	comedi_t *dev;

	fn = "/dev/comedi0";

	dev = comedi_open(fn);

	do_cmd(dev);

	return 0;
}

static void do_cmd(comedi_t *dev)
{
	comedi_cmd cmd;
	unsigned int chanlist[4];
	int ret;

	/* the subdevice that the command is sent to */
	cmd.subdev =	subdevice;

	/* flags */
	cmd.flags =	0;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

	cmd.start_src =		TRIG_NOW;
	cmd.start_arg =		0;

	cmd.scan_begin_src =	TRIG_TIMER;
	cmd.scan_begin_arg =	1000000;	/* in ns */

	cmd.convert_src =	TRIG_TIMER;
	cmd.convert_arg =	100000;		/* in ns */

	cmd.scan_end_src =	TRIG_COUNT;
	cmd.scan_end_arg =	4;		/* number of channels */

	cmd.stop_src =		TRIG_COUNT;
	cmd.stop_arg =		1000;

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd.chanlist =		chanlist;
	cmd.chanlist_len =	4;

	chanlist[0]=CR_PACK(0,range,aref);
	chanlist[1]=CR_PACK(1,range,aref);
	chanlist[2]=CR_PACK(2,range,aref);
	chanlist[3]=CR_PACK(3,range,aref);

	/* data and data_len are not used for user-space
	   programs */
	cmd.data = NULL;
	cmd.data_len = 0;

	ret=ioctl(comedi_fileno(dev),COMEDI_CMD,&cmd);

	printf("ioctl returned %d\n",ret);

	do{
		ret=read(comedi_fileno(dev),buf,BUFSZ);
		printf("read %d\n",ret);
	}while(ret>=0);
	printf("errno=%d\n",errno);
}

