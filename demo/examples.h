
#ifndef _EXAMPLES_H
#define _EXAMPLES_H

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

int parse_options(int argc, char *argv[]);

#endif

