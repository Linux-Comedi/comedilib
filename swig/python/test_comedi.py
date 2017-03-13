#!/usr/bin/env python
from __future__ import print_function  # Python3 compatibility
"""
A test-application to demonstrate using the Comedilib API
for input, output, and commands.

Original author bryan.cole@teraview.co.uk
Updated by Joseph T. Foley <foley at RU dot IS>

Source in https://github.com/Linux-Comedi/comedilib
at swig/python/test_comedi.py
Documentation in doc/bindings.xml and
http://www.comedi.org/doc/languagebindings.html

Debian/Ubuntu Install from packages:
sudo apt install comedilib-dev python-comedilib python3-comedilib
sudo usermod -a -G iocard <USER>  # Local user access

As of 2017-03-12, the debian packages have broken backwards
compatibility, which means you should use open() instead of comedi_open().
This example assumes that backwards compatibility is broken.

If you are building comedilib yourself, you can turn backwards
compatibility on by editing comedilib/swig/comedi.i and commenting out

#define SWIGPYTHONONLYSHORT

This code has been tested on a Velleman 2006 P8061-2.

"""

import sys
import os
import string
import time
import argparse
# set the paths so python can find the comedi module
# Only needed if the C library can't be found, which seems to happen on pytnon3
if (sys.version_info > (3, 0)):  # python3
    sys.path.append('/usr/lib/python3/dist-packages/comedi')

import comedi as c
# This import is later  because the path may need to be appended.


class COMEDILIBtest(object):
    'Test comedilib functionality'

    def __init__(self, devpath='/dev/comedi0'):
        # open a comedi device
        dev = c.open(devpath)
        if not dev:
            raise IOError("Error opening Comedi device: %s" % devpath)
        self.dev = dev
        # get a file-descriptor for use later
        self.cfd = c.fileno(dev)

    def scancommand(self, nscans=1000, chans=None, gains=None, aref=None):
        """Setup a scan
        nscans: total number of scans (int)
        chans: which channels (list)
        gains: which gain adjustments (list)
        aref: analog references (list)
        The lists must all have the same length!

        [sic] I think this will work but it's completely untested!
        --bryan

        I was not able to get this to work with my Vellemen P8061-2
        board -- foley
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
        cmd = c.cmd_struct()
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
        ret = c.command_test(dev, cmd)
        print("first cmd test returns ", ret)
        ret = c.command_test(dev, cmd)
        print("second test returns ", ret)
        ret = c.command_test(dev, cmd)
        if not ret:
            raise RuntimeError("Error testing comedi command")

        ret = c.command(self.dev, cmd)

        chunk = 8
        # chunk: how many bytes to read at a time
        datalist = []
        data = os.read(self.cfd, chunk)
        while len(data) > 0:
            datalist.append(data)
            data = os.read(self.cfd, chunk)

        datastr = string.join(datalist, '')
        print("Data from command:")
        print(datastr)  # Here's your data, as a single string!
        # if you've got Numeric installed you can convert data
        # into a flat Numpy array with:
        # dataarray = Numeric.fromstring(data, Int16)
        return ret

    def read_analog(
            self, subdev, chan, iorange=1, aref=c.AREF_GROUND):
        """Setup a scan
        subdev: which subdevice (int)
        chan: which channel (int)
        iorange: gain adjustments (int)
        aref: analog references (int)
        """

        retcode, data = c.data_read(self.dev, subdev, chan, iorange, aref)
        if retcode != 1:
            raise IOError("data_read returned %s" % retcode)
        return data

    def write_analog(
            self, subdev, chan, value, iorange=1, aref=c.AREF_GROUND):
        """Setup a scan
        subdev: which subdevice (int)
        chan: which channel (int)
        iorange: gain adjustments (int)
        aref: analog references (int)
        """

        retcode = c.data_write(self.dev, subdev, chan, iorange, aref, value)
        return retcode

    def write_digital(self, subdev, chan, bit):
        """Write a value to a digital channel"""
        retcode = c.dio_write(self.dev, subdev, chan, bit)
        return retcode

    def read_digital(self, subdev, chan):
        """Read a value from a digital channel"""
        retcode, data = c.dio_read(self.dev, subdev, chan)
        if retcode != 1:
            raise IOError("dio_read returned %s" % retcode)
        return data

    def close(self):
        'close and cleanup'
        ret = c.close(self.dev)
        return ret

if __name__ == "__main__":
    # pylint: disable=invalid-name
    # disable pylint because it thinks all top level variables
    # are constants.
    basedir = os.path.abspath(os.path.dirname(sys.argv[0]))

    parser = argparse.ArgumentParser(
        description='comedilib python SWIG tests',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    # puts the defaults in the help automatically!
    parser.add_argument('-d', '--debug', action='store_true',
                        help='Set debug flags')
    parser.add_argument('-i', '--input', default='/dev/comedi0',
                        help='comedi device')

    args = parser.parse_args()
    cet = COMEDILIBtest(args.input)

    # NSCANS = 100
    # CHANS = [0, 1, 3]
    # GAINS = [1, 1, 1]
    # AREF = [c.AREF_GROUND, c.AREF_GROUND, c.AREF_GROUND]
    # cet.scancommand(NSCANS, CHANS, GAINS, AREF)

    # Quick digital test on a Velleman P8061
    # Connect Digital in pin 1 to Digital out pin 1.
    # Connect Analog in pin 1 to Analog out pin 1.
    # Of note, on the Velleman P8061, the digital lights will
    # alternate because the outputs are open-collector (short to
    # ground on enable)
    AIN = 0
    AOUT = 1
    DIN = 2
    DOUT = 3
    COUNTER = 4
    PWM = 5

    print("Testing digital inputs and outputs.")
    for _ in range(3):
        print("Setting chan %d to value 1" % DOUT)
        cet.write_digital(DOUT, 0, 1)
        print("Channel %d is %s" % (DIN, cet.read_digital(2, 0)))
        time.sleep(0.1)
        print("Setting chan %d to value 0" % DOUT)
        cet.write_digital(DOUT, 0, 0)
        print("Channel %d is %s" % (DIN, cet.read_digital(DIN, 0)))
        time.sleep(0.1)

    print("Testing analog inputs and outputs.")
    ahigh = 200
    for _ in range(3):
        print("Setting chan %d to value %d" % (AOUT, ahigh))
        cet.write_analog(AOUT, 0, ahigh)
        print("Channel %d is %s" % (AIN, cet.read_analog(AIN, 0)))
        time.sleep(0.1)
        print("Setting chan %d to value 0" % AOUT)
        cet.write_analog(AOUT, 0, 0)
        print("Channel %d is %s" % (AIN, cet.read_analog(AIN, 0)))
        time.sleep(0.1)

    cet.close()
