#include "comedilib_scxi.h"

int comedi_scxi_serial_config(comedi_t *it, unsigned int clock_interval)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn,0,sizeof(insn));
	insn.insn = INSN_CONFIG;
	insn.n = 2;
	insn.data = &data[0];
	insn.subdev = comedi_find_subdevice_by_type(it, COMEDI_SUBD_SERIAL, 0);
	data[0]=INSN_CONFIG_SERIAL_CLOCK;
	data[1]=clock_interval;

	return comedi_do_insn(it,&insn);
}

int comedi_scxi_serial_readwrite(comedi_t *it, unsigned char out_bits, unsigned char *in_bits)
{
	int ret;
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn,0,sizeof(insn));

	insn.insn = INSN_CONFIG;
	insn.n = 2;
	insn.data = data;
	insn.subdev = comedi_find_subdevice_by_type(it, COMEDI_SUBD_SERIAL, 0);

	data[0]=INSN_CONFIG_BIDIRECTIONAL_DATA;
	data[1]=out_bits;

	ret = comedi_do_insn(it,&insn);

	if(ret<0) return ret;

	if(in_bits)
		*in_bits = data[1];

	return 0;
}
