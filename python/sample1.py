#! /usr/bin/env python
#
# Sample1.py sample code to use python with comedi via the compy interface.
# Compy: COMedi PYthon interface
#
# Blaine Lee Copyright 11/2000 Licence GPL 2.0
#
# V1 reworkd sample.py to test a number of the new functions added to 
#    compy.c.  Seems to work with the das16 driver.  John Conner - 20020304
# V0 hacked out of working code for others to look at.

############# imports
import os
import stat
import time
import comedi			# important if you want to use compy
from string import *

board = 0
sub_dev = 0
brd_dev = (board,sub_dev)
channel = 0

debug_level = 0

comedi.open(board,"/dev/comedi0",debug_level)
print 'The compy versions is ', comedi.__version__()

driver_name = comedi.get_driver_name(board)
board_name  = comedi.get_board_name(board)
num_subdevices = comedi.get_n_subdevices(board)
print 'The board uses driver %s, is named %s and has %d subdevices'%(
			driver_name,board_name,num_subdevices)
for subdevice in range(0,num_subdevices):
    print '\tsubdevice %d is type %d'%(
    		subdevice,comedi.get_subdevice_type(board,subdevice))
print 'Subdevice %d has %d channels'%(
		sub_dev,comedi.get_n_channels(board,sub_dev))

maxdata = comedi.get_maxdata(board,sub_dev,channel)
print 'The maximum input count for channel %d is %d'%(channel,maxdata)
num_ranges = comedi.get_n_ranges(board,sub_dev,channel)
print 'Channel %d has %d ranges'%(channel,num_ranges)
for rng in range(0,num_ranges):
    (min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
    if unit == 0: unit = 'volts'
    if unit == 1: unit = 'mAmps'
    if unit == 2: unit = ''
    print '\trange %d  %8.3f  -- %8.3f  %s'%(rng,min,max,unit)

rng = 0
(min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
val = comedi.data_read(brd_dev,channel,rng,0);
volt = comedi.to_phys(val,(min,max,unit),maxdata);
print 'range = %d  input = %6d (%6d) or %8.4f volts'%(rng, val, val-32768, volt)

rng = 1
(min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
val = comedi.data_read(brd_dev,channel,rng,0);
volt = comedi.to_phys(val,(min,max,unit),maxdata);
print 'range = %d  input = %6d (%6d) or %8.4f volts'%(rng, val, val-32768, volt)

rng = 2
(min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
val = comedi.data_read(brd_dev,channel,rng,0);
volt = comedi.to_phys(val,(min,max,unit),maxdata);
print 'range = %d  input = %6d (%6d) or %8.4f volts'%(rng, val, val-32768, volt)

rng = 3
(min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
val = comedi.data_read(brd_dev,channel,rng,0);
volt = comedi.to_phys(val,(min,max,unit),maxdata);
print 'range = %d  input = %6d (%6d) or %8.4f volts'%(rng, val, val-32768, volt)

rng = 1
(min,max,unit) = comedi.get_range(board,sub_dev,channel,rng)
volt = 1.5
val = comedi.from_phys(volt,(min,max,unit),maxdata)
print 'range = %d   an input of %8.4f volts should give %6d from A/D' % (rng, volt, val)

comedi.close(0)

