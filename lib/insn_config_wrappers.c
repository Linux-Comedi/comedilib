/*
    lib/insn_config_wrappers.c
    wrappers for various INSN_CONFIG instructions

    COMEDILIB - Linux Control and Measurement Device Interface Library
    Copyright (C) 1997-2001 David A. Schleef <ds@schleef.org>
    Copyright (C) 2008 Frank Mori Hess <fmhess@users.sourceforge.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA.
*/

#include <string.h>

#include "libinternal.h"

EXPORT_ALIAS_DEFAULT(_comedi_reset,comedi_reset,0.9.0);
int _comedi_reset(comedi_t *device, unsigned subdevice)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_RESET;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_arm,comedi_arm,0.9.0);
int _comedi_arm(comedi_t *device, unsigned subdevice, unsigned target)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_ARM;
	data[1] = target;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_counter_mode,comedi_set_counter_mode,0.9.0);
int _comedi_set_counter_mode(comedi_t *device, unsigned subdevice, unsigned mode_bits)
{
	comedi_insn insn;
	lsampl_t data[2];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_COUNTER_MODE;
	data[1] = mode_bits;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_clock_source,comedi_set_clock_source,0.9.0);
int _comedi_set_clock_source(comedi_t *device, unsigned subdevice, unsigned clock, unsigned period_ns)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_CLOCK_SRC;
	data[1] = clock;
	data[2] = period_ns;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_gate_source,comedi_set_gate_source,0.9.0);
int _comedi_set_gate_source(comedi_t *device, unsigned subdevice, unsigned gate_index, unsigned gate_source)
{
	comedi_insn insn;
	lsampl_t data[3];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_GATE_SRC;
	data[1] = gate_index;
	data[2] = gate_source;

	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_internal_trigger,comedi_internal_trigger,0.9.0);
int comedi_internal_trigger(comedi_t *dev, unsigned subd, unsigned trignum)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_INTTRIG;
	insn.subdev = subd;
	insn.data = data;
	insn.n = 1;

	data[0] = trignum;

	if(comedi_do_insn(dev, &insn) >= 0) return 0;
	else return -1;
}

EXPORT_ALIAS_DEFAULT(_comedi_set_other_source,comedi_set_other_source,0.9.0);
int _comedi_set_other_source(comedi_t *device, unsigned subdevice,
	unsigned other, unsigned source)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = 0;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_OTHER_SRC;
	data[1] = other;
	data[2] = source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	if(comedi_do_insn(device, &insn) >= 0) return 0;
	else return -1;
}
