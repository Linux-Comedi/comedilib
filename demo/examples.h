
#ifndef _EXAMPLES_H
#define _EXAMPLES_H

#include <stdio.h>

/*
 * Definitions of some of the common code.
 */

extern char *filename;
extern int verbose_flag;
extern comedi_t *device;

extern int value;
extern int subdevice;
extern int channel;
extern int aref;
extern int range;
extern int verbose;
extern int n_chan;
extern int n_scan;
extern double freq;

int parse_options(int argc, char *argv[]);
char *cmd_src(int src,char *buf);
void dump_cmd(FILE *file,comedi_cmd *cmd);


#define sec_to_nsec(x) ((x)*1000000000)
#define sec_to_usec(x) ((x)*1000000)
#define sec_to_msec(x) ((x)*1000)
#define msec_to_nsec(x) ((x)*1000000)
#define msec_to_usec(x) ((x)*1000)
#define usec_to_nsec(x) ((x)*1000)

#endif

