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
#include <string.h>

#include "comedi_test.h"

char *filename="/dev/comedi0";
int verbose_flag;
comedi_t *device;

int subdevice;
int channel;
int aref;
int range;

int test_info(void);
int test_mode0_read(void);
int test_insn_read(void);
int test_insn_read_time(void);
int test_cmd_probe_src_mask(void);
int test_cmd_probe_fast_1chan(void);
int test_cmd_read_fast_1chan(void);
int test_cmd_fifo_depth_check(void);
int test_mmap(void);

struct test_struct{
	char *name;
	int (*do_test)(void);
};
struct test_struct tests[]={
	{ "info", test_info },
	{ "mode0_read", test_mode0_read },
	{ "insn_read", test_insn_read },
	{ "insn_read_time", test_insn_read_time },
	{ "cmd_probe_src_mask", test_cmd_probe_src_mask },
	{ "cmd_probe_fast_1chan", test_cmd_probe_fast_1chan },
	{ "cmd_read_fast_1chan", test_cmd_read_fast_1chan },
	{ "cmd_fifo_depth_check", test_cmd_fifo_depth_check },
	{ "mmap", test_mmap },
};
static int n_tests = sizeof(tests)/sizeof(tests[0]);

int main(int argc, char *argv[])
{
	int c;
	int i;

	while (1) {
		c = getopt(argc, argv, "f");
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			filename = argv[optind];
			break;
		default:
			printf("bad option\n");
			exit(1);
		}
	}

	device = comedi_open(filename);
	if(!device){
		printf("E: comedi_open(\"%s\"): %s\n",filename,strerror(errno));
	}

	for(subdevice=0;subdevice<comedi_get_n_subdevices(device);subdevice++){
		printf("I:\n");
		printf("I: subdevice %d\n",subdevice);
		for(i=0;i<n_tests;i++){
			printf("I: testing %s...\n",tests[i].name);
			tests[i].do_test();
		}
	}

	return 0;
}



