
#include <stdio.h>      /* for printf() */
#include <comedi.h>     /* also included by comedilib.h */
#include <comedilib.h>  /* for comedi_get() */

int subdev = 0;         /* change this to your input subdevice */
int chan = 0;           /* change this to your channel */
int range = 3;          /* more on this later */
int aref = 0;           /* more on this later */

int main(int argc,char *argv[])
{
        comedi_t *cf;
        int chan=0;
        int data;
        int maxdata;
        double volts;
	comedi_range *cr;

        cf=comedi_open("/dev/comedi0");

        maxdata=comedi_get_maxdata(cf,subdev,chan);

	cr=comedi_get_range(cf,subdev,chan,range);

        comedi_data_read(cf,subdev,chan,range,aref,&data);

        volts=comedi_to_phys(data,cr,maxdata);

        printf("%d %g\n",data,volts);

        return 0;
}
