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
import comedi			# important if you want to use compy
from string import *

comedi.open(0,"/dev/comedi0",1)

val = comedi.read_data((0,0,0));
print val

comedi.close(0)

