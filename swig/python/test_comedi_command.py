#!/usr/bin/env python
"""
A test-application to demonstrate using the Comedilib API
and streaming acquisition in particular, from Python.

Original author bryan.cole@teraview.co.uk
Tested and updated by Joseph T. Foley <foley at RU dot IS>

Source in https://github.com/Linux-Comedi/comedilib
at swig/python/test_comedi.py

DEBIAN:
sudo apt-get install comedilib-dev python3-comedilib python-comedilib
sudo usermod -a -G iocard <USER>

As of 2017-03-12, the debian packages have broken backwards
compatibility, which means you should use open() instead of comedi_open().
This example assumes that backwards compatibility is broken.

If you are building comedilib yourself, you can turn backwards
compatibility on by editing comedilib/swig/comedi.i and commenting out

#define SWIGPYTHONONLYSHORT


"""

import sys
import os
import string
import argparse

# set the paths so python can find the comedi module
# Only needed if the C library can't be found, which seems to happen on pytnon3
if (sys.version_info > (3, 0)):  # python3
    sys.path.append('/usr/lib/python3/dist-packages/comedi')

import comedi as c
# This import is later  because the path may need to be appended.


class COMEDILIBtest(object):
    'Test comedilib command functionality'

    def __init__(self, devpath='/dev/comedi0'):
        # open a comedi device
        dev = c.open(devpath)
        if not dev:
            raise IOError("Error opening Comedi device: %s" % devpath)
        self.dev = dev
        # get a file-descriptor for use later
        self.cfd = c.fileno(dev)
        self.cmd = None

    def scancommand(self, nscans=1000, chans=None, gains=None, aref=None):
        """Setup a scan
        nscans: total number of scans (int)
        chans: which channels (list)
        gains: which gain adjustments (list)
        aref: analog references (list)
        The lists must all have the same length!
        """
        if not chans:
            raise RuntimeError("No Channels given in 'chans'")
        if not gains:
            raise RuntimeError("No gains given in 'gains'")
        if not aref:
            raise RuntimeError("No analog references given in 'aref'")
        if not len(chans) == len(gains) == len(aref):
            raise RuntimeError(
                "chans(%d), gains(%d), and aref(%d) not the same size"
                % (len(chans), len(gains), len(aref)))

        nchans = len(chans)  # number of channels

        # wrappers include a "chanlist" object (just an Unsigned Int
        # array) for holding the chanlist information
        mylist = c.chanlist(nchans)  # create a chanlist of length nchans

        # now pack the channel, gain and reference information into
        # the chanlist object
        # N.B. the CR_PACK and other comedi macros are now python functions
        for index in range(nchans):
            mylist[index] = c.cr_pack(
                chans[index], gains[index], aref[index])

        # construct a comedi command manually
        # TODO: make helper methods
        cmd = c.comedi_cmd_struct()
        cmd.subdev = 0
        cmd.flags = 0
        cmd.start_src = c.TRIG_NOW
        cmd.sart_arg = 0
        cmd.scan_begin_src = c.TRIG_TIMER
        cmd.scan_begin_arg = int(1.0E9 / 100000)
        cmd.convert_src = c.TRIG_TIMER
        cmd.convert_arg = 1
        cmd.scan_end_src = c.TRIG_COUNT
        cmd.scan_end_arg = nchans
        cmd.stop_src = c.TRIG_COUNT
        cmd.stop_arg = nscans
        cmd.chanlist = mylist
        cmd.chanlist_len = nchans

        dev = self.dev
        # test our comedi command a few times.
        ret = c.comedi_command_test(dev, cmd)
        print("first cmd test returns ", ret)
        ret = c.comedi_command_test(dev, cmd)
        print("second test returns ", ret)
        ret = c.comedi_command_test(dev, cmd)
        if not ret:
            raise "Error testing comedi command"
        self.cmd = cmd

        ret = c.comedi_command(self.dev, self.cmd)
        return ret

    def readcommand(self):
        """Read some data from a running command
        [sic] I think this will work but it's completely untested! --bryan
        """
        datalist = []
        data = os.read(self.cfd)
        while len(data) > 0:
            datalist.append(data)
            data = os.read(self.cfd)

        datastr = string.join(datalist, '')

        print(datastr)  # Here's your data, as a single string!
        # if you've got Numeric installed you can convert data
        # into a flat Numpy array with:
        # dataarray = Numeric.fromstring(data, Int16)

    def close(self):
        'close and cleanup'
        ret = c.comedi_close(self.dev)
        return ret

if __name__ == "__main__":
    BASEDIR = os.path.abspath(os.path.dirname(sys.argv[0]))

    PARSER = argparse.ArgumentParser(
        description='comedilib python SWIG command test',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    # puts the defaults in the help automatically!
    PARSER.add_argument('-d', '--debug', action='store_true',
                        help='Set debug flags')
    PARSER.add_argument('-i', '--input', default='/dev/comedi0',
                        help='comedi device, default /dev/comedi0')

    ARGS = PARSER.parse_args()
    CET = COMEDILIBtest(ARGS.input)
    NSCANS = 1000
    CHANS = [0, 2, 3]
    GAINS = [c.AREF_GROUND, c.AREF_GROUND, c.AREF_GROUND]
    CET.prepscan(NSCANS, CHANS, GAINS)
    CET.execute()
    CET.read()
    CET.close()
