/*  Compy: COMedi PYthon interface
* A simple hack sharded object for importing into Python
* to start using Comedi drivers for Digital I/O
* 
* Extensively referenced xxmodule.c and others from python distribution
*                        misc comedi examples.
*
* Blaine Lee Copyright 11/2000 Licence LGPL 2.0
*
* V 0.3  changed to tuple parsing & added version number __version__()
* V 0.21 Resynced with sorce that had printstats debugging code in it
* V 0.2  Added card number, so that multiple cards are supported
* V 0.1  First release, only 'trig' type transfers are supported 
*
*/

#include <Python.h>
#include <stdio.h>	/* for printf() */
#include <comedilib.h>

#define maxcards 4

static short trigdata;
static comedi_t *compy_it[maxcards];
static comedi_trig trig;
static int trigchan[2];
static printstats=1;

/*******************************/
static PyObject *
compy_open(self, args)
	PyObject *self;
	PyObject *args;
{
	const char *filen;
	int card;

	if (!PyArg_ParseTuple(args, "isi", &card, &filen, &printstats))
		return NULL;
	if ((card < 0) || (card >= maxcards))
		return NULL;

	if(printstats)
		printf("compy open '%s'\n",filen);

	compy_it[card]=comedi_open(filen);

	trig.data=&trigdata;
	trig.mode=0;
	trig.flags=0;
	trig.n_chan=1;
	trig.chanlist=trigchan;
	trig.n=1;
	trig.trigsrc=TRIG_NOW;
	trig.data_len=1;

	return Py_BuildValue("i", 1);
}

static PyObject *
compy_trig(self, args)
	PyObject *self;
	PyObject *args;
{
	int dev, chan, data;
	int card;

	if (!PyArg_ParseTuple(args, "(iii)i", &card, &dev, &chan, &data))
		return NULL;
	if ((card < 0) || (card >= maxcards))
		return NULL;
	if(printstats)
		printf("compy trig card %d dev %d chanel %d val %d\n",card,dev,chan,data);

	trig.subdev=dev;
	trig.chanlist[0]=chan;
	trig.data[0]=data;

	comedi_trigger(compy_it[card],&trig);

	return Py_BuildValue("i", trig.data[0]);
}

static PyObject *
compy_close(self, args)
	PyObject *self;
	PyObject *args;
{
	int card;

	if (!PyArg_ParseTuple(args, "i", &card))
		return NULL;
	if ((card < 0) || (card >= maxcards))
		return NULL;
	if(printstats)
		printf("compy close %d\n",card);
	comedi_close(compy_it[card]);

	return Py_BuildValue("i", 1);
}

#define Version 3

static PyObject *
compy_version(self, args)
	PyObject *self;
	PyObject *args;
{
	if(printstats)
		printf("Compy version %d\n",Version);

	return Py_BuildValue("i", Version);
}


/* List of functions defined in the module */

static PyMethodDef compy_methods[] = {
        {"open",	compy_open,	METH_VARARGS},
        {"trig",	compy_trig,	METH_VARARGS},
        {"close",	compy_close,	METH_VARARGS},
        {"__version__",	compy_version,	METH_VARARGS},
        {NULL,		NULL}           /* sentinel */
};

/* Initialization function for the module (*must* be called initxx) */

DL_EXPORT(void)
initcompy()
{
        /* Create the module and add the functions */
        (void) Py_InitModule("compy", compy_methods);
}


