/*
   This demo reads information about a comedi device and
   displays the information in a human-readable form.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

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
		n_chans=comedi_get_n_channels(it,i);
		printf("  number of channels: %d\n",n_chans);
		printf("  max data value: %d\n",comedi_get_maxdata(it,i,0));
		n_ranges=comedi_get_n_ranges(it,i,0);
		printf("  number of ranges: %d\n",n_ranges);
		printf("  ranges:\n");
		for(chan=0;chan<n_chans;chan++){
			printf("    chan%d:",chan);
			//printf(" (0x%08x)",comedi_get_rangetype(it,i,chan));
			for(j=0;j<n_ranges;j++){
				rng=comedi_get_range(it,i,chan,j);
				//printf(" %p",rng);
				printf(" [%g,%g]",rng->min,rng->max);
			}
			printf("\n");
		}
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

