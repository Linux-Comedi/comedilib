/*
   This program reads information about a comedi device and
   displays the information in a human-readable form.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


static char * const default_filename = "/dev/comedi0";

int verbose = 0;

static const char * const subdevice_types[]={
	"unused",
	"analog input",
	"analog output",
	"digital input",
	"digital output",
	"digital I/O",
	"counter",
	"timer",
	"memory",
	"calibration",
	"processor",
	"serial digital I/O",
	"pwm"
};

struct subdev_flag {
	const char *sdf_define;
	unsigned int bitmask;
	const char *description;
};

static struct subdev_flag subdev_flags[] = {
	{"SDF_MAXDATA",0x0010,"maxdata depends on channel"},
	{"SDF_FLAGS",0x0020,"flags depend on channel"},
	{"SDF_RANGETYPE",0x0040,"range type depends on channel"},
	{"SDF_MODE0",0x0080,"can do mode 0"},
	{"SDF_MODE1",0x0100,"can do mode 1"},
	{"SDF_MODE2",0x0200,"can do mode 2"},
	{"SDF_MODE3",0x0400,"can do mode 3"},
	{"SDF_MODE4",0x0800,"can do mode 4"},
	{"SDF_SOFT_CALIBRATED",0x2000,"subdevice uses software calibration"},
	{"SDF_CMD_WRITE",0x4000,"can do asynchronous output commands"},
	{"SDF_CMD_READ",0x8000,"can do asynchronous input commands"},
	{"SDF_READABLE",0x00010000,"subdevice can be read"},
	{"SDF_WRITABLE",0x00020000,"subdevice can be written"},
	{"SDF_INTERNAL",0x00040000,"subdevice does not have externally visible lines"},
	{"SDF_GROUND",0x00100000,"can do aref=ground"},
	{"SDF_COMMON",0x00200000,"can do aref=common"},
	{"SDF_DIFF",0x00400000,"aref=diff"},
	{"SDF_OTHER",0x00800000,"can do aref=other"},
	{"SDF_DITHER",0x01000000,"can do dithering"},
	{"SDF_DEGLITCH",0x02000000,"can do deglitching"},
	{"SDF_MMAP",0x04000000,"can do mmap()"},
	{"SDF_RUNNING",0x08000000,"subdevice is acquiring data"},
	{"SDF_LSAMPL",0x10000000,"subdevice uses 32-bit samples for commands"},
	{"SDF_PACKED",0x20000000,"subdevice can do packed DIO"},
	{0,0,0}};

void explain_subdevice_flags(char* padding,unsigned int sf) {
	int i = 0;
	while (subdev_flags[i].sdf_define) {
		if (sf & subdev_flags[i].bitmask)
			printf("%s%s:%s\n",
			       padding,
			       subdev_flags[i].sdf_define,
			       subdev_flags[i].description);
		i++;
	}
}

void unit_to_desc(char *udesc,int unit) {
	switch(unit) {
	case UNIT_volt: strcpy(udesc," V"); break;
	case UNIT_mA: strcpy(udesc," mA"); break;
	case UNIT_none: strcpy(udesc,""); break; 
	default: sprintf(udesc," (unknown unit %d)",
			 unit);
	}
}



char *tobinary(char *s,int bits,int n)
{
	int bit=1<<n;
	char *t=s;

	for(;bit;bit>>=1)
		*t++=(bits&bit)?'1':'0';
	*t=0;

	return s;
}


char *cmd_src(int src,char *buf)
{
	buf[0]=0;

	if(src&TRIG_NONE)strcat(buf,"none|");
	if(src&TRIG_NOW)strcat(buf,"now|");
	if(src&TRIG_FOLLOW)strcat(buf, "follow|");
	if(src&TRIG_TIME)strcat(buf, "time|");
	if(src&TRIG_TIMER)strcat(buf, "timer|");
	if(src&TRIG_COUNT)strcat(buf, "count|");
	if(src&TRIG_EXT)strcat(buf, "ext|");
	if(src&TRIG_INT)strcat(buf, "int|");
#ifdef TRIG_OTHER
	if(src&TRIG_OTHER)strcat(buf, "other|");
#endif

	if(strlen(buf)==0){
		sprintf(buf,"unknown(0x%08x)",src);
	}else{
		buf[strlen(buf)-1]=0;
	}

	return buf;
}



void probe_cmd_generic_timed(comedi_t *it,int s,int n_channels,int freq_for_generic_timed)
{
	comedi_cmd cmd;
	char buf[100];

	printf("  command structure filled with probe_cmd_generic_timed for %d channels:\n",
	       n_channels);
	if(comedi_get_cmd_generic_timed(it, s, &cmd, n_channels, 1E9/freq_for_generic_timed)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s %d\n",
			cmd_src(cmd.start_src,buf),cmd.start_arg);
		printf("    scan_begin: %s %d\n",
			cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
		if (verbose) {
			if ((cmd.scan_begin_src == TRIG_TIMER)&&(cmd.scan_begin_arg)) {
				printf("      scan_begin_src = TRIG_TIMER:\n"
				       "      The sampling rate is defined per scan\n"
				       "      meaning all channels are sampled at\n"
				       "      the same time. The maximum sampling rate is f=%d Hz\n",
				       (int)(1E9/cmd.scan_begin_arg));}
		}
		printf("    convert: %s %d\n",
			cmd_src(cmd.convert_src,buf),cmd.convert_arg);
		if (verbose) {
			if ((cmd.convert_src == TRIG_TIMER)&&(cmd.convert_arg)) {
				printf("      convert_src = TRIG_TIMER\n"
				       "      The sampling rate is defined per channel\n"
				       "      meaning that a multiplexer is being switched from\n"
				       "      channel to channel at a maximum rate of %d Hz.\n"
				       "      The overall sampling rate needs to be divided\n"
				       "      by the number of channels and results in f=%d Hz.\n",
				       (int)(1E9/cmd.convert_arg),
				       (int)(1E9/cmd.convert_arg/n_channels));
			}
		}
		printf("    scan_end: %s %d\n",
			cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
		printf("    stop: %s %d\n",
			cmd_src(cmd.stop_src,buf),cmd.stop_arg);
	}
}



void get_command_stuff(comedi_t *it,int s,int n_chans_for_generic_timed,int freq_for_generic_timed)
{
	comedi_cmd cmd;
	char buf[100];

	if(comedi_get_cmd_src_mask(it,s,&cmd)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s\n",cmd_src(cmd.start_src,buf));
		if (cmd.start_src == TRIG_EXT)
			printf("    cmd.start_src allows external trigger (TRIG_EXT),"
			       "    for example from on input pin at the device.\n");
		printf("    scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
		printf("    convert: %s\n",cmd_src(cmd.convert_src,buf));
		printf("    scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
		printf("    stop: %s\n",cmd_src(cmd.stop_src,buf));

		probe_cmd_generic_timed(it,s,n_chans_for_generic_timed,freq_for_generic_timed);
	}
}


		
int main(int argc,char *argv[])
{
	int i,j;
	int n_subdevices,type;
	const char *type_str;
	int chan,n_chans;
	int n_ranges;
	int subdev_flags;
	comedi_range *rng;
	comedi_t *it;
	char *filename = default_filename;
	char c;
	char strtmp[32];
	int def_n_chans_for_generic_timed = 1;
	int n_chans_for_generic_timed;
	int freq_for_generic_timed = 1E9;

	while (-1 != (c = getopt(argc, argv, "hvn:F:"))) {
		switch (c) {
		case 'n':
			def_n_chans_for_generic_timed = strtoul(optarg, NULL, 0);
			break;
		case 'F':
			freq_for_generic_timed = strtoul(optarg, NULL, 0);
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
		default:
			fprintf(stderr,
				"usage: comedi_board_info [OPTIONS] COMEDI_DEVICE\n"
				"   -n    number of channels for async command (default 1)\n"
				"   -F    probing sampling rate for async command (default 1Ghz)\n"
				"   -v    verbose output\n"
				"   -h    this help screen\n");
			exit(1);
		}
	}

	if(optind < argc) {
		filename = argv[optind];
	}

	it = comedi_open(filename);
	if(!it){
		comedi_perror(filename);
		exit(1);
	}

	printf("overall info:\n");
	printf("  version code: 0x%06x\n", comedi_get_version_code(it));
	printf("  driver name: %s\n", comedi_get_driver_name(it));
	printf("  board name: %s\n", comedi_get_board_name(it));
	printf("  number of subdevices: %d\n", n_subdevices = comedi_get_n_subdevices(it));

	for(i = 0; i < n_subdevices; i++){
		printf("subdevice %d:\n",i);
		type = comedi_get_subdevice_type(it, i);
		if(type < (int)(sizeof(subdevice_types) / sizeof(subdevice_types[0]))){
			type_str = subdevice_types[type];
		}else{
			type_str = "UNKNOWN";
		}
		printf("  type: %d (%s)\n",type,type_str);
		if(type==COMEDI_SUBD_UNUSED)
			continue;
		subdev_flags = comedi_get_subdevice_flags(it, i);
		printf("  flags: 0x%08x\n",subdev_flags);
		if (verbose) explain_subdevice_flags("          ",subdev_flags);
		n_chans=comedi_get_n_channels(it,i);
		printf("  number of channels: %d\n",n_chans);
		if(!comedi_maxdata_is_chan_specific(it,i)){
			printf("  max data value: %lu\n", (unsigned long)comedi_get_maxdata(it,i,0));
		}else{
			printf("  max data value: (channel specific)\n");
			for(chan=0;chan<n_chans;chan++){
				printf("    chan%d: %lu\n",chan,
					(unsigned long)comedi_get_maxdata(it,i,chan));
			}
		}
		printf("  ranges:\n");
		if(!comedi_range_is_chan_specific(it,i)){
			n_ranges=comedi_get_n_ranges(it,i,0);
			printf("    all chans:");
			for(j=0;j<n_ranges;j++){
				rng=comedi_get_range(it,i,0,j);
				unit_to_desc(strtmp,rng->unit);
				printf(" [%g%s,%g%s]",rng->min,strtmp,rng->max,strtmp);
			}
			printf("\n");
		}else{
			for(chan=0;chan<n_chans;chan++){
				n_ranges=comedi_get_n_ranges(it,i,chan);
				printf("    chan%d:",chan);
				for(j=0;j<n_ranges;j++){
					rng=comedi_get_range(it,i,chan,j);
					unit_to_desc(strtmp,rng->unit);
					printf(" [%g%s,%g%s]",rng->min,strtmp,rng->max,strtmp);
				}
				printf("\n");
			}
		}
		printf("  command:\n");
		n_chans_for_generic_timed = def_n_chans_for_generic_timed;
		if (n_chans_for_generic_timed>n_chans)
			n_chans_for_generic_timed = n_chans;
		if (n_chans_for_generic_timed<1)
			n_chans_for_generic_timed = 1;
		if (freq_for_generic_timed > 1E9)
			freq_for_generic_timed = 1E9;
		if (freq_for_generic_timed < 1)
			freq_for_generic_timed = 1;
		get_command_stuff(it,i,n_chans_for_generic_timed,freq_for_generic_timed);
	}

	return 0;
}

