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
int test_cmd_no_cmd(void);
int test_cmd_probe_src_mask(void);
int test_cmd_probe_fast_1chan(void);
int test_cmd_read_fast_1chan(void);
int test_cmd_logic_bug(void);
int test_cmd_fifo_depth_check(void);
int test_mmap(void);
int test_read_select(void);
int test_cmd_continuous(void);
int test_bufconfig(void);

#define TEST_NEVER 0
#define TEST_STD 1

struct test_struct{
	char *name;
	int (*do_test)(void);
	int flags;
};
struct test_struct tests[]={
	{ "info", test_info, TEST_STD },
	{ "mode0_read", test_mode0_read, TEST_STD },
	{ "insn_read", test_insn_read, TEST_STD },
	{ "insn_read_time", test_insn_read_time, TEST_STD },
	{ "cmd_no_cmd", test_cmd_no_cmd, TEST_STD },
	{ "cmd_probe_src_mask", test_cmd_probe_src_mask, TEST_STD },
	{ "cmd_probe_fast_1chan", test_cmd_probe_fast_1chan, TEST_STD },
	{ "cmd_read_fast_1chan", test_cmd_read_fast_1chan, TEST_STD },
	{ "cmd_logic_bug", test_cmd_logic_bug, TEST_STD },
	{ "cmd_fifo_depth_check", test_cmd_fifo_depth_check, TEST_STD },
	{ "mmap", test_mmap, TEST_STD },
	{ "read_select", test_read_select, TEST_STD },
	{ "cmd_continuous", test_cmd_continuous, TEST_NEVER },
	{ "bufconfig", test_bufconfig, TEST_STD },
};
static int n_tests = sizeof(tests)/sizeof(tests[0]);

int only_subdevice;
int verbose;
char *only_test;

static void get_capabilities(unsigned int subd);
static void print_device_info(void);

int main(int argc, char *argv[])
{
	int c;
	int i;

	while (1) {
		c = getopt(argc, argv, "f:s:t:v");
		if (c == -1)
			break;
		switch (c) {
		case 'f':
			filename = optarg;
			break;
		case 's':
			only_subdevice = 1;
			sscanf(optarg,"%d",&subdevice);
			break;
		case 't':
			only_test = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("bad option\n");
			exit(1);
		}
	}

	device = comedi_open(filename);
	if(!device){
		printf("E: comedi_open(\"%s\"): %s\n",filename,strerror(errno));
		exit(0);
	}

	print_device_info();

	for(;subdevice<comedi_get_n_subdevices(device);subdevice++){
		printf("I:\n");
		printf("I: subdevice %d\n",subdevice);
		get_capabilities(subdevice);
		if(only_test){
			for(i=0;i<n_tests;i++){
				if(!strcmp(tests[i].name,only_test)){
					printf("I: testing %s...\n",tests[i].name);
					tests[i].do_test();
				}
			}
		}else{
			for(i=0;i<n_tests;i++){
				if(tests[i].flags&TEST_STD){
					printf("I: testing %s...\n",tests[i].name);
					tests[i].do_test();
				}
			}
		}
		if(only_subdevice)break;
	}

	return 0;
}

unsigned int capabilities;

static void get_capabilities(unsigned int subd)
{
	int type;
	int flags;

	capabilities = 0;

	type = comedi_get_subdevice_type(device,subd);

	flags = comedi_get_subdevice_flags(device,subd);

}

static void print_device_info(void)
{
	int vers = comedi_get_version_code(device);

	printf("I: Comedi version: %d.%d.%d\n",(vers>>16)&0xff,
		(vers>>8)&0xff,vers&0xff);
	printf("I: Comedilib version: unknown =)\n");
	printf("I: driver name: %s\n",comedi_get_driver_name(device));
	printf("I: device name: %s\n",comedi_get_board_name(device));
}



