/*
   This demo reads information about a comedi device and
   displays the information in a human-readable form.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "examples.h"

void get_command_stuff(comedi_t *it,int s);

void help(void)
{
	fprintf(stderr,"info </dev/comediN>\n");
	exit(0);
}

char *tobinary(char *s,int bits,int n);

char *subdevice_types[]={
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
	"processor"
};

comedi_t *it;
extern char *filename;


int main(int argc,char *argv[])
{
	int i,j;
	int n_subdevices,type;
	int chan,n_chans;
	int n_ranges;
	comedi_range *rng;
	
	parse_options(argc,argv);

	it=comedi_open(filename);
	if(!it){
		fprintf(stderr,"cannot open %s\n",filename);
		exit(0);
	}

	printf("overall info:\n");
	printf("  version code: 0x%06x\n",comedi_get_version_code(it));
	printf("  driver name: %s\n",comedi_get_driver_name(it));
	printf("  board name: %s\n",comedi_get_board_name(it));
	printf("  number of subdevices: %d\n",n_subdevices=comedi_get_n_subdevices(it));
	
	for(i=0;i<n_subdevices;i++){
		printf("subdevice %d:\n",i);
		type=comedi_get_subdevice_type(it,i);
		printf("  type: %d (%s)\n",type,subdevice_types[type]);
		if(type==COMEDI_SUBD_UNUSED)
			continue;
		n_chans=comedi_get_n_channels(it,i);
		printf("  number of channels: %d\n",n_chans);
		if(!comedi_maxdata_is_chan_specific(it,i)){
			printf("  max data value: %d\n",comedi_get_maxdata(it,i,0));
		}else{
			printf("  max data value: (channel specific)\n");
			for(chan=0;chan<n_chans;chan++){
				printf("    chan%d: %d\n",chan,
					comedi_get_maxdata(it,i,chan));
			}
		}
		printf("  ranges:\n");
		if(!comedi_range_is_chan_specific(it,i)){
			n_ranges=comedi_get_n_ranges(it,i,0);
			printf("    all chans:");
			for(j=0;j<n_ranges;j++){
				rng=comedi_get_range(it,i,0,j);
				printf(" [%g,%g]",rng->min,rng->max);
			}
			printf("\n");
		}else{
			for(chan=0;chan<n_chans;chan++){
				n_ranges=comedi_get_n_ranges(it,i,chan);
				printf("    chan%d:",chan);
				for(j=0;j<n_ranges;j++){
					rng=comedi_get_range(it,i,chan,j);
					printf(" [%g,%g]",rng->min,rng->max);
				}
				printf("\n");
			}
		}
		printf("  command:\n");
		get_command_stuff(it,i);
	}
	
	return 0;
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

void probe_max_1chan(comedi_t *it,int s);

int comedi_get_cmd_src_mask(comedi_t *it,unsigned int s,comedi_cmd *cmd)
{
	memset(cmd,0,sizeof(*cmd));

	cmd->subdev = s;

	cmd->flags = 0;

	cmd->start_src = TRIG_ANY;
	cmd->scan_begin_src = TRIG_ANY;
	cmd->convert_src = TRIG_ANY;
	cmd->scan_end_src = TRIG_ANY;
	cmd->stop_src = TRIG_ANY;

	return comedi_command_test(it,cmd);
}


int comedi_get_cmd_fast_1chan(comedi_t *it,unsigned int s,comedi_cmd *cmd)
{
	int ret;

	comedi_get_cmd_src_mask(it,s,cmd);

	cmd->chanlist_len = 1;

	cmd->scan_end_src = TRIG_COUNT;
	cmd->scan_end_arg = 1;

	if(cmd->convert_src&TRIG_TIMER){
		if(cmd->scan_begin_src&TRIG_FOLLOW){
			cmd->convert_src = TRIG_TIMER;
			cmd->scan_begin_src = TRIG_FOLLOW;
		}else{
			cmd->convert_src = TRIG_TIMER;
			cmd->scan_begin_src = TRIG_TIMER;
		}
	}else{
		printf("can't do timed?!?\n");
		return -1;
	}
	if(cmd->stop_src&TRIG_COUNT){
		cmd->stop_src=TRIG_COUNT;
		cmd->stop_arg=2;
	}else if(cmd->stop_src&TRIG_NONE){
		cmd->stop_src=TRIG_NONE;
		cmd->stop_arg=0;
	}else{
		printf("can't find a good stop_src\n");
		return -1;
	}

	ret=comedi_command_test(it,cmd);
	if(ret==3){
		/* good */
		ret=comedi_command_test(it,cmd);
	}
	if(ret==4 || ret==0){
		return 0;
	}
	return -1;
}

void get_command_stuff(comedi_t *it,int s)
{
	comedi_cmd cmd;
	char buf[100];

	if(comedi_get_cmd_src_mask(it,s,&cmd)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s\n",cmd_src(cmd.start_src,buf));
		printf("    scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
		printf("    convert: %s\n",cmd_src(cmd.convert_src,buf));
		printf("    scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
		printf("    stop: %s\n",cmd_src(cmd.stop_src,buf));
	
		probe_max_1chan(it,s);
	}
}

void probe_max_1chan(comedi_t *it,int s)
{
	comedi_cmd cmd;
	char buf[100];

	printf("  command fast 1chan:\n");
	if(comedi_get_cmd_fast_1chan(it,s,&cmd)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s %d\n",
			cmd_src(cmd.start_src,buf),cmd.start_arg);
		printf("    scan_begin: %s %d\n",
			cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
		printf("    convert: %s %d\n",
			cmd_src(cmd.convert_src,buf),cmd.convert_arg);
		printf("    scan_end: %s %d\n",
			cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
		printf("    stop: %s %d\n",
			cmd_src(cmd.stop_src,buf),cmd.stop_arg);
	}
}

