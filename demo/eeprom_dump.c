/*
   Dumps eeproms
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <stdlib.h>
#include "examples.h"

int read_eeprom(comedi_t *it,unsigned int **eeprom);
void dump_eeprom(unsigned int *eeprom,int len);

comedi_t *device;

int main(int argc, char *argv[])
{
	int len;
	unsigned int *eeprom;

	parse_options(argc,argv);

	device=comedi_open(filename);
	if(!device){
		comedi_perror(filename);
		exit(0);
	}

	len=read_eeprom(device,&eeprom);
	dump_eeprom(eeprom,len);

	return 0;
}




int read_eeprom(comedi_t *it,unsigned int **eeprom)
{
	int subd;
	int n,i,ret;
	lsampl_t data;
	unsigned int *ptr;
	lsampl_t maxdata;

	subd=comedi_find_subdevice_by_type(it,COMEDI_SUBD_MEMORY,0);
	if(subd<0){
		fprintf(stderr,"No memory subdevice\n");
		return 0;
	}

	n=comedi_get_n_channels(it,subd);
	maxdata=comedi_get_maxdata(it,subd,0);

	if(maxdata!=0xff){
		fprintf(stderr,"Memory subdevice has strange maxdata, aborting\n");
	}

	ptr=malloc(sizeof(unsigned int)*n);

	for(i=0;i<n;i++){
		ret=comedi_data_read(it,subd,i,0,0,&data);
		ptr[i]=data;
		if(ret<0){
			comedi_perror("comedi_data_read");
			return 0;
		}
	}

	*eeprom=ptr;
	return n;

}

void dump_eeprom(unsigned int *eeprom,int len)
{
	int i, j, c;

	for (i = 0; i < len - 16; i+=16) {
		printf("%04x: ",i);
		for (j = 0; j < 16; j++) {
			printf("%02x", eeprom[i + j] & 0xff);
		}
		printf("  ");
		for (j = 0; j < 16; j++) {
			c = eeprom[i + j] & 0xff;
			printf("%c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
	if(i==len)return;
	printf("%04x: ",i);
	for (j = 0; j < len-i; j++) {
		printf("%02x", eeprom[i + j] & 0xff);
	}
	for(;j<16;j++){
		printf("  ");
	}
	printf("  ");
	for (j = 0; j < len-i; j++) {
		c = eeprom[i + j] & 0xff;
		printf("%c", isprint(c) ? c : '.');
	}
	for(;j<16;j++){
		printf(" ");
	}
	printf("\n");
}

