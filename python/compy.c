/*  Compy: COMedi PYthon interface
* A simple hack sharded object for importing into Python
* to start using Comedi drivers for Digital I/O
* 
* Extensively referenced xxmodule.c and others from python distribution
*                        misc comedi examples.
*
* Blaine Lee Copyright 11/2000 Licence LGPL 2.0
*
* V 0.4  Major additions to add support for more of the comedilib
* 	 functions.  So far we have support for the comedi_ commands
* 	 open, close, get_n_subdevices, get_driver_name, get_board_name
* 	 get_n_channels, get_maxdata, get_n_ranges, get_range, 
* 	 to_phys, from_phys, data_read.   Changed the way the debug
* 	 and printstatus code works. 
* V 0.3  changed to tuple parsing & added version number __version__()
* V 0.21 Resynced with sorce that had printstats debugging code in it
* V 0.2  Added card number, so that multiple cards are supported
* V 0.1  First release, only 'trig' type transfers are supported 
*
*/

#include <Python.h>
#include <stdio.h>	/* for printf() */
#include <comedilib.h>

#define Version 4

#define maxcards 4

static comedi_t *compy_it[maxcards];
#ifdef _COMEDILIB_DEPRECATED
static comedi_trig trig;
static int trigchan[2];
static short trigdata;
#endif
/******************************
 * Debug flags - set with the 
 * open call. Output is sent
 * to stderr.
 *****************************/
static int printstats=0;	// if 1 prints mostly the received calling parms
static int debug = 0;		// if 1 print details of function calls

/******************************
 * compy_open()
 * opens the requested comedi device
 * args - a tuple containing
 * 	int - board number (0 to maxcards)
 * 	string - the comedi device to open 
 * 	int - printstats 0-no, 1-yes, 2-also debug
 ******************************/
static PyObject *
compy_open( PyObject *self, PyObject *args )
{
    const char *filen;
    int card;

    if (!PyArg_ParseTuple(args, "isi", &card, &filen, &printstats))
	return NULL;
    if (printstats > 1)
	debug = 1;

    if ((card < 0) || (card >= maxcards))
	return NULL;

    if(printstats)
	fprintf(stderr,"compy open '%s'\n",filen);

    compy_it[card]=comedi_open(filen);

    if(debug)
	fprintf(stderr,"open returned %p\n",compy_it[card]);

    return Py_BuildValue("i", 1);
}

/******************************
 * compy_close()
 * close the requested comedi device
 * args - a tuple containing
 * 	int - board number (0 to maxcards)
 ******************************/
static PyObject *
compy_close( PyObject *self, PyObject *args )
{
    int card;

    if (!PyArg_ParseTuple(args, "i", &card))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy close %d\n",card);
    comedi_close(compy_it[card]);

    return Py_BuildValue("i", 1);
}

/******************************
 * compy_get_n_subdevices()
 * get the number of subdevices
 * args -
 * 	int - board number (0 to maxcards)
 * returns the number of subdevices on card
 ******************************/
static PyObject *
compy_get_n_subdevices( PyObject *self, PyObject *args )
{
    int card;

    if (!PyArg_ParseTuple(args, "i:get_n_subdevices", &card))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_n_subdevices %d\n",card);

    return Py_BuildValue("i", comedi_get_n_subdevices(compy_it[card]));
}

/******************************
 * compy_get_driver_name()
 * get the number of subdevices
 * args -
 * 	int - board number (0 to maxcards)
 * returns the name of the driver for the board
 ******************************/
static PyObject *
compy_get_driver_name( PyObject *self, PyObject *args )
{
    int card;

    if (!PyArg_ParseTuple(args, "i:get_driver_name", &card))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_driver_name (%d)\n",card);

    return Py_BuildValue("s", comedi_get_driver_name(compy_it[card]));
}

/******************************
 * compy_get_board_name()
 * get the number of subdevices
 * args -
 * 	int - board number (0 to maxcards)
 * returns the name of the driver for the board
 ******************************/
static PyObject *
compy_get_board_name( PyObject *self, PyObject *args )
{
    int card;

    if (!PyArg_ParseTuple(args, "i:get_board_name", &card))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_board_name (%d)\n",card);

    return Py_BuildValue("s", comedi_get_board_name(compy_it[card]));
}

/******************************
 * compy_get_subdevice_type()
 * get the number of subdevices
 * args -
 * 	int - board number (0 to maxcards)
 * returns the name of the driver for the board
 ******************************/
static PyObject *
compy_get_subdevice_type( PyObject *self, PyObject *args )
{
    int card;
    int sub_dev;

    if (!PyArg_ParseTuple(args, "ii:get_subdevice_type", &card,&sub_dev))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_subdevice_type (%d)\n",card);

    return Py_BuildValue("i", comedi_get_subdevice_type(compy_it[card],sub_dev));
}

/******************************
 * compy_get_n_channels()
 * get the number of channels on subdevice
 * args - 
 * 	int - board number (0 to maxcards)
 * 	int - subdevice
 * returns number of channels on subdevice
 ******************************/
static PyObject *
compy_get_n_channels( PyObject *self, PyObject *args )
{
    int card;
    int sub_dev;

    if (!PyArg_ParseTuple(args, "ii", &card, &sub_dev))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_n_channels ( %d, %d)\n",card,sub_dev);

    return Py_BuildValue("i", comedi_get_n_channels(compy_it[card],sub_dev));
}

/******************************
 * compy_get_maxdata()
 * get the maxdata value for a channel
 * args -
 * 	int - board number (0 to maxcards)
 * 	int - subdevice number
 * 	int - channel number
 * returns maximum sample value for channel (0 on error)
 ******************************/
static PyObject *
compy_get_maxdata( PyObject *self, PyObject *args )
{
    int card;
    int sub_dev;
    int channel;

    if (!PyArg_ParseTuple(args, "iii:maxdata", &card,&sub_dev,&channel))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_maxdata ( %d, %d, %d)\n",card,sub_dev,channel);

    return Py_BuildValue("i", comedi_get_maxdata(compy_it[card],sub_dev,channel));
}

/******************************
 * compy_get_n_ranges()
 * get the number of ranges a channel
 * args -
 * 	int - board number (0 to maxcards)
 * 	int - subdevice number
 * 	int - channel number
 * returns number of ranges for channel (-1 on error)
 ******************************/
static PyObject *
compy_get_n_ranges( PyObject *self, PyObject *args )
{
    int card;
    int sub_dev;
    int channel;

    if (!PyArg_ParseTuple(args, "iii:get_n_ranges", &card,&sub_dev,&channel))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_n_ranges ( %d, %d, %d)\n",card,sub_dev,channel);

    return Py_BuildValue("i", comedi_get_n_ranges(compy_it[card],sub_dev,channel));
}

/******************************
 * compy_get_range()
 * get the number of ranges a channel
 * args -
 * 	int - board number (0 to maxcards)
 * 	int - subdevice number
 * 	int - channel number
 * returns range information on channel 
 * 	tuple containing 
 *         min value
 *         max value
 *         units ( 0-volts, 1-mAmps, 2-none)
 ******************************/
static PyObject *
compy_get_range( PyObject *self, PyObject *args )
{
    comedi_range *rng_p;
    int card;
    int sub_dev;
    int channel;
    int range;

    if (!PyArg_ParseTuple(args, "iiii:get_range", &card,&sub_dev,&channel,&range))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_n_ranges ( %d, %d, %d, %d)\n",card,sub_dev,channel,range);

    if ((rng_p = comedi_get_range(compy_it[card],sub_dev,channel,range))==NULL)
	return NULL;
    else
	return Py_BuildValue("ddi", rng_p->min, rng_p->max, rng_p->unit);
}

/******************************
 * compy_to_phys()
 * convert sample to physical units
 * args -
 * 	int - data
 * 	tuple containing range information
 * 	    min value
 * 	    max value
 * 	    units ( 0-volts, 1-mAmps, 2-none)
 * 	int - maxdata value for channel
 * returns physical value (volts) for input
 ******************************/
static PyObject *
compy_to_phys( PyObject *self, PyObject *args )
{
    comedi_range rng;
    lsampl_t data;
    lsampl_t maxdata;

    if (!PyArg_ParseTuple(args, "i(ddi)i:to_phys",
		&data,&rng.min,&rng.max,&rng.unit,&maxdata))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_to_phys ( %d, %8.3f, %8.3f, %d, %d)\n",
		data,rng.min,rng.max,rng.unit,maxdata);

    return Py_BuildValue("d",comedi_to_phys(data,&rng, maxdata) );
}

/******************************
 * compy_from_phys()
 * convert physical units to sample
 * args -
 * 	int - data
 * 	tuple containing range information
 * 	    min value
 * 	    max value
 * 	    units ( 0-volts, 1-mAmps, 2-none)
 * 	int - maxdata value for channel
 * returns the sample value (0-maxdata)
 ******************************/
static PyObject *
compy_from_phys( PyObject *self, PyObject *args )
{
    comedi_range rng;
    double data;
    lsampl_t maxdata;

    if (!PyArg_ParseTuple(args, "d(ddi)i:from_phys",
		&data,&rng.min,&rng.max,&rng.unit,&maxdata))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy get_from_phys ( %8.3f, %8.3f, %8.3f, %d, %d)\n",
		data,rng.min,rng.max,rng.unit,maxdata);

    return Py_BuildValue("i",comedi_from_phys(data,&rng, maxdata) );
}
/******************************
 * compy_data_read()
 * read the requested comedi device
 * args -
 *      tuple containing
 * 	    int - board number (0 to maxcards)
 * 	    int - subdevice number
 * 	int - channel number
 * 	int - range
 * 	int - aref
 ******************************/
static PyObject *
compy_data_read(PyObject *self, PyObject *args)
{
    int card;
    int subd, chan;
    unsigned int range = 0;
    unsigned int aref  = 0;
    lsampl_t data;

    if (!PyArg_ParseTuple(args, "(ii)i|ii:data_read", &card, &subd, &chan,&range,&aref))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(debug)
	fprintf(stderr,"compy_data_read dev %d subd %d chan %d range %d aref %d\n",card,subd,chan,range,aref);

    comedi_data_read(compy_it[card],subd,chan,range,aref,&data);

    if(debug)
	fprintf(stderr,"comedi_data_read value %d\n",data);

    return Py_BuildValue("l", data);
}

#ifdef _COMEDILIB_DEPRECATED
/******************************
 * compy_trig()
 * read the requested comedi device
 * args - a tuple containing
 * 	int - board number (0 to maxcards)
 * 	int - subdevice number
 * 	int - channel number
 ******************************/
static PyObject *
compy_trig( PyObject *self, PyObject *args )
{
    int dev, chan, data;
    int card;

    if (!PyArg_ParseTuple(args, "(iii)i", &card, &dev, &chan, &data))
	return NULL;
    if ((card < 0) || (card >= maxcards))
	return NULL;
    if(printstats)
	fprintf(stderr,"compy trig card %d dev %d chanel %d val %d\n",card,dev,chan,data);

    trig.subdev=dev;
    trig.chanlist[0]=chan;
    trig.data[0]=data;

    comedi_trigger(compy_it[card],&trig);

    return Py_BuildValue("i", trig.data[0]);
}
#endif


static PyObject *
compy_version( PyObject *self, PyObject *args )
{
    if(printstats)
	fprintf(stderr,"Compy version %d\n",Version);

    return Py_BuildValue("i", Version);
}


/* List of functions defined in the module */

static PyMethodDef comedi_methods[] = {
    {"open",		compy_open,		METH_VARARGS},
    {"close",		compy_close,		METH_VARARGS},
    {"get_n_subdevices",compy_get_n_subdevices,	METH_VARARGS},
    {"get_driver_name",	compy_get_driver_name,	METH_VARARGS},
    {"get_board_name",	compy_get_board_name,	METH_VARARGS},
    {"get_subdevice_type",compy_get_subdevice_type,	METH_VARARGS},
    {"get_n_channels",	compy_get_n_channels,	METH_VARARGS},
    {"get_maxdata",	compy_get_maxdata,	METH_VARARGS},
    {"get_n_ranges",	compy_get_n_ranges,	METH_VARARGS},
    {"get_range",	compy_get_range,	METH_VARARGS},
#ifdef _COMEDILIB_DEPRECATED
    {"trig",		compy_trig,		METH_VARARGS},
#endif
    {"to_phys",		compy_to_phys,		METH_VARARGS},
    {"from_phys",	compy_from_phys,	METH_VARARGS},
    {"data_read",	compy_data_read,	METH_VARARGS},
    {"__version__",	compy_version,		METH_VARARGS},
    {NULL,		NULL}           /* sentinel */
};

static char comedi_doc[] = "Module to interface comedi library to Python\n";

/* Initialization function for the module (*must* be called initxx) */

DL_EXPORT(void)
initcomedi(void)
{
    /* Create the module and add the functions */
    (void) Py_InitModule3("comedi", comedi_methods, comedi_doc);
}


