#! /usr/bin/env python

# Sample.py sample code to use python with comedi via the compy interface.
# Compy: COMedi PYthon interface
#
# Blaine Lee Copyright 11/2000 Licence GPL 2.0
#
# V0 hacked out of working code for others to look at.

############# imports
import os
import stat
import time
import compy			# important if you want to use compy
from string import *

#  this project uses 3 advantech PCM-3730's
#  8 isolated outputs on dev 0
#  8 isolated inputs on dev 3
#  8 ttl outputs each on dev 1 & 2 not used
#  8 ttl inputs each on dev 4 & 5 not used

compypath0 = "/dev/comedi0"	# a little abstraction
compypath1 = "/dev/comedi1"
compypath2 = "/dev/comedi2"

# ************ I/O definitions
SolderSOV	= (0,0,1)
Work0SOV	= (0,0,2)	# input card 0, dev 0, channel 2
Work1SOV	= (0,0,3)
Move1SOV	= (0,0,4)
BlowSOV		= (0,0,5)
SolderDn	= (0,3,2)
SolderUp	= (0,3,3)
Work0In		= (0,3,4)
Work0Out	= (0,3,5)
SolderPresent0	= (0,3,6)
Work1In		= (1,3,0)
Work1Out	= (1,3,1)
SolderPresent1	= (1,3,2)
MoveAt1		= (1,3,3)
MoveAt0		= (1,3,4)
PalmL		= (2,3,0)
PalmR		= (2,3,1)	# input card 2, dev 3, channel 1
PartPres0	= (2,3,2)
PartPres1	= (2,3,3)

compyif = compy.trig		# got rid of old work around of compy v2
				# all new code should use compy.trig directly!!!

# *********************************************
def inout(SOV, Sin, Sout, Ppres, Head ):
    "A task to manage one arm of a 'ping-pong' machine"
    global SolderNow
    compyif(SOV,0)	# reset
    SolderNow[Head] = 0
    while 1:
        while not compyif(Sout,0):	# wait until out and ready
            time.sleep(.001)
        while compyif(Ppres,0):		# Wait for part to be removed
            time.sleep(.001)
        while not compyif(Ppres,0):	# Wait for part to be replaced
            time.sleep(.001)
        while compyif(PalmL,0) or compyif(PalmR,0):	# palms must not be blocked
            time.sleep(.001)
        while (not compyif(PalmL,0)) or (not compyif(PalmR,0)):	# wait for palms
            time.sleep(.001)
        SolderNow[Head] = 1		# signal that this side will be ready soon
        compyif(SOV,1)			# go in
        while not compyif(Sin,0):	# wait for in
            time.sleep(.001)
        SolderNow[Head] = 2
        while SolderNow[Head]:		# wait for process
            time.sleep(.001)
            # Of course this won't exit, you don't have the other tasks that run
            # the machine!!!!
        compyif(SOV,0)			# go out


# ************ comedi setup
compy.open(0,compypath0,0)
compy.open(1,compypath1,0)
compy.open(2,compypath2,0)

compyif(SolderSOV,0)	# solder up

inout(Work0SOV,Work0In,Work0Out,PartPres0,0)

compy.close(0)
compy.close(1)
compy.close(2)

