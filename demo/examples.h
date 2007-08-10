
#ifndef _EXAMPLES_H
#define _EXAMPLES_H

#include <stdio.h>

/*
 * Definitions of some of the common code.
 */

extern comedi_t *device;

struct parsed_options
{
	char *filename;
	double value;
	int subdevice;
	int channel;
	int aref;
	int range;
	int physical;
	int verbose;
	int n_chan;
	int n_scan;
	double freq;
};

extern void init_parsed_options(struct parsed_options *options);
extern int parse_options(struct parsed_options *options, int argc, char *argv[]);
extern char *cmd_src(int src,char *buf);
extern void dump_cmd(FILE *file,comedi_cmd *cmd);
/* some helper functions used primarily for counter demos */
extern int arm(comedi_t *device, unsigned subdevice, lsampl_t source);
extern int reset_counter(comedi_t *device, unsigned subdevice);
extern int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits);
extern int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns);
extern int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source);
extern int comedi_internal_trigger(comedi_t *dev, unsigned int subd, unsigned int trignum);

#define sec_to_nsec(x) ((x)*1000000000)
#define sec_to_usec(x) ((x)*1000000)
#define sec_to_msec(x) ((x)*1000)
#define msec_to_nsec(x) ((x)*1000000)
#define msec_to_usec(x) ((x)*1000)
#define usec_to_nsec(x) ((x)*1000)

#endif

