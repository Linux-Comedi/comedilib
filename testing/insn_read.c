
#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "comedi_test.h"


int test_insn_read(void)
{
	comedi_insn it;
	lsampl_t data;
	int save_errno;
	int ret;

	printf("rev 1\n");

	memset(&it,0,sizeof(it));
	it.subdev = subdevice;
	it.insn = INSN_READ;
	it.n = 1;
	it.chanspec = CR_PACK(0,0,0);
	it.data = &data;

	ret = comedi_do_insn(device,&it);
	save_errno = errno;

	printf("comedi_do_insn: %d\n",ret);
	if(ret<0){
		printf("W: comedi_do_insn: errno=%d %s\n",save_errno,strerror(save_errno));
	}

	return 0;
}

