

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <rtl_sched.h>
#include <asm/rt_time.h>
#include <comedi.h>


RT_TASK mytask;

/* this is the dev,subdev for digital I/O on my atmio-E board */
unsigned int dev=0;
unsigned int subdev=2;

unsigned int channel;
sampl_t data;
comedi_trig trig;

void do_comedi_toggle(int t)
{
	while(1){
		data^=1;
		comedi_trig_ioctl(dev,subdev,&trig);
		rt_task_wait();
	}
}

int init_module(void)
{
	int ret;
	RTIME now=rt_get_time();

	/* set up trigger structure */
	trig.subdev=subdev;
	trig.mode=0;
	trig.flags=0;
	trig.n_chan=1;
	trig.chanlist=&channel;
	trig.data=&data;
	trig.n=1;
	trig.trigsrc=0;
	trig.trigvar=0;
	trig.trigvar1=0;

	channel=CR_PACK(0,0,0);

	/* IMPORTANT next step: lock the subdevice */
	comedi_lock_ioctl(dev,subdev);

	/* configure DIO 0 for output */
	trig.flags=TRIG_CONFIG;
	data=COMEDI_OUTPUT;
	ret=comedi_trig_ioctl(dev,subdev,&trig);
	printk("comedi_trig_ioctl() returned %d\n",ret);
	trig.flags=0;
	data=1;

	/* a little test */
	ret=comedi_trig_ioctl(dev,subdev,&trig);
	printk("comedi_trig_ioctl() returned %d\n",ret);

	rt_task_init(&mytask,do_comedi_toggle, 0xffff, 3000, 4);
	
	rt_task_make_periodic(&mytask,now+3000,50000);

	return 0;
}

void cleanup_module(void)
{
	rt_task_delete(&mytask);

	comedi_unlock_ioctl(dev,subdev);
}


