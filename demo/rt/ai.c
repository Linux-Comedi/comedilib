

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <rtl_sched.h>
#include <asm/rt_time.h>
#include <comedi.h>


RT_TASK mytask;

/* this is the dev,subdev for analog input on my atmio-E board */
unsigned int ai_dev=0;
unsigned int ai_subdev=0;
unsigned int ai_chan[2]={CR_PACK(6,0,0),CR_PACK(1,0,0)};

comedi_trig ai_trig;

/* this is the dev,subdev for analog output on my atmio-E board */
unsigned int ao_dev=0;
unsigned int ao_subdev=1;
unsigned int ao_chan=CR_PACK(0,0,0);

comedi_trig ao_trig;

sampl_t data2=0;

sampl_t data[2];


int callback(void *arg)
{
	data2^=0x800;
	data[0]^=0x800;
	data[0]&=0xfff;

	comedi_trig_ioctl(ao_dev,ao_subdev,&ao_trig);

	/* returning 1 is a little hack to reset user_ptr */
	return 1;
}

int init_module(void)
{
	int ret;

	/* set up input structure */
	ai_trig.subdev=ai_subdev;
	ai_trig.mode=2;
	ai_trig.flags=0;
	ai_trig.n_chan=2;
	ai_trig.chanlist=ai_chan;
	ai_trig.data=data;
	ai_trig.n=2000;
	ai_trig.trigsrc=0;
	ai_trig.trigvar=99999;
	ai_trig.trigvar1=1999;

	/* IMPORTANT next step: lock the subdevice */
	comedi_lock_ioctl(ai_dev,ai_subdev);

	/* register our callback function */
	ret=comedi_register_callback(ai_dev,ai_subdev,COMEDI_CB_EOS,callback,(void *)0);
	printk("comedi_register_callback() returned %d\n",ret);

	/* set up output structure */
	ao_trig.subdev=ao_subdev;
	ao_trig.mode=0;
	ao_trig.flags=0;
	ao_trig.n_chan=1;
	ao_trig.chanlist=&ao_chan;
	ao_trig.data=data;
	ao_trig.n=1;
	ao_trig.trigsrc=0;
	ao_trig.trigvar=0;
	ao_trig.trigvar1=0;

	/* IMPORTANT next step: lock the subdevice */
	comedi_lock_ioctl(ao_dev,ao_subdev);

	/* start acq. */
	ret=comedi_trig_ioctl(ai_dev,ai_subdev,&ai_trig);
	printk("comedi_trig_ioctl() returned %d\n",ret);

	return 0;
}

void cleanup_module(void)
{
	comedi_cancel_ioctl(ai_dev,ai_subdev);

	comedi_unlock_ioctl(ai_dev,ai_subdev);
	comedi_unlock_ioctl(ao_dev,ao_subdev);
}


