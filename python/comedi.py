# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.
import _comedi
def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


cr_pack = _comedi.cr_pack

cr_pack_flags = _comedi.cr_pack_flags

cr_chan = _comedi.cr_chan

cr_range = _comedi.cr_range

cr_aref = _comedi.cr_aref

class chanlist(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, chanlist, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, chanlist, name)
    def __init__(self,*args):
        _swig_setattr(self, chanlist, 'this', apply(_comedi.new_chanlist,args))
        _swig_setattr(self, chanlist, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_chanlist):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __getitem__(*args): return apply(_comedi.chanlist___getitem__,args)
    def __setitem__(*args): return apply(_comedi.chanlist___setitem__,args)
    def cast(*args): return apply(_comedi.chanlist_cast,args)
    __swig_getmethods__["frompointer"] = lambda x: _comedi.chanlist_frompointer
    if _newclass:frompointer = staticmethod(_comedi.chanlist_frompointer)
    def __repr__(self):
        return "<C chanlist instance at %s>" % (self.this,)

class chanlistPtr(chanlist):
    def __init__(self,this):
        _swig_setattr(self, chanlist, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, chanlist, 'thisown', 0)
        _swig_setattr(self, chanlist,self.__class__,chanlist)
_comedi.chanlist_swigregister(chanlistPtr)
chanlist_frompointer = _comedi.chanlist_frompointer


COMEDI_MAJOR = _comedi.COMEDI_MAJOR
COMEDI_NDEVICES = _comedi.COMEDI_NDEVICES
COMEDI_NDEVCONFOPTS = _comedi.COMEDI_NDEVCONFOPTS
COMEDI_NAMELEN = _comedi.COMEDI_NAMELEN
CR_FLAGS_MASK = _comedi.CR_FLAGS_MASK
CR_ALT_FILTER = _comedi.CR_ALT_FILTER
CR_DITHER = _comedi.CR_DITHER
CR_DEGLITCH = _comedi.CR_DEGLITCH
CR_ALT_SOURCE = _comedi.CR_ALT_SOURCE
CR_EDGE = _comedi.CR_EDGE
CR_INVERT = _comedi.CR_INVERT
AREF_GROUND = _comedi.AREF_GROUND
AREF_COMMON = _comedi.AREF_COMMON
AREF_DIFF = _comedi.AREF_DIFF
AREF_OTHER = _comedi.AREF_OTHER
GPCT_RESET = _comedi.GPCT_RESET
GPCT_SET_SOURCE = _comedi.GPCT_SET_SOURCE
GPCT_SET_GATE = _comedi.GPCT_SET_GATE
GPCT_SET_DIRECTION = _comedi.GPCT_SET_DIRECTION
GPCT_SET_OPERATION = _comedi.GPCT_SET_OPERATION
GPCT_ARM = _comedi.GPCT_ARM
GPCT_DISARM = _comedi.GPCT_DISARM
GPCT_GET_INT_CLK_FRQ = _comedi.GPCT_GET_INT_CLK_FRQ
GPCT_INT_CLOCK = _comedi.GPCT_INT_CLOCK
GPCT_EXT_PIN = _comedi.GPCT_EXT_PIN
GPCT_NO_GATE = _comedi.GPCT_NO_GATE
GPCT_UP = _comedi.GPCT_UP
GPCT_DOWN = _comedi.GPCT_DOWN
GPCT_HWUD = _comedi.GPCT_HWUD
GPCT_SIMPLE_EVENT = _comedi.GPCT_SIMPLE_EVENT
GPCT_SINGLE_PERIOD = _comedi.GPCT_SINGLE_PERIOD
GPCT_SINGLE_PW = _comedi.GPCT_SINGLE_PW
GPCT_CONT_PULSE_OUT = _comedi.GPCT_CONT_PULSE_OUT
GPCT_SINGLE_PULSE_OUT = _comedi.GPCT_SINGLE_PULSE_OUT
INSN_MASK_WRITE = _comedi.INSN_MASK_WRITE
INSN_MASK_READ = _comedi.INSN_MASK_READ
INSN_MASK_SPECIAL = _comedi.INSN_MASK_SPECIAL
INSN_READ = _comedi.INSN_READ
INSN_WRITE = _comedi.INSN_WRITE
INSN_BITS = _comedi.INSN_BITS
INSN_CONFIG = _comedi.INSN_CONFIG
INSN_GTOD = _comedi.INSN_GTOD
INSN_WAIT = _comedi.INSN_WAIT
INSN_INTTRIG = _comedi.INSN_INTTRIG
TRIG_BOGUS = _comedi.TRIG_BOGUS
TRIG_DITHER = _comedi.TRIG_DITHER
TRIG_DEGLITCH = _comedi.TRIG_DEGLITCH
TRIG_CONFIG = _comedi.TRIG_CONFIG
CMDF_PRIORITY = _comedi.CMDF_PRIORITY
TRIG_RT = _comedi.TRIG_RT
TRIG_WAKE_EOS = _comedi.TRIG_WAKE_EOS
CMDF_WRITE = _comedi.CMDF_WRITE
TRIG_WRITE = _comedi.TRIG_WRITE
CMDF_RAWDATA = _comedi.CMDF_RAWDATA
COMEDI_EV_START = _comedi.COMEDI_EV_START
COMEDI_EV_SCAN_BEGIN = _comedi.COMEDI_EV_SCAN_BEGIN
COMEDI_EV_CONVERT = _comedi.COMEDI_EV_CONVERT
COMEDI_EV_SCAN_END = _comedi.COMEDI_EV_SCAN_END
COMEDI_EV_STOP = _comedi.COMEDI_EV_STOP
TRIG_ROUND_MASK = _comedi.TRIG_ROUND_MASK
TRIG_ROUND_NEAREST = _comedi.TRIG_ROUND_NEAREST
TRIG_ROUND_DOWN = _comedi.TRIG_ROUND_DOWN
TRIG_ROUND_UP = _comedi.TRIG_ROUND_UP
TRIG_ROUND_UP_NEXT = _comedi.TRIG_ROUND_UP_NEXT
TRIG_ANY = _comedi.TRIG_ANY
TRIG_INVALID = _comedi.TRIG_INVALID
TRIG_NONE = _comedi.TRIG_NONE
TRIG_NOW = _comedi.TRIG_NOW
TRIG_FOLLOW = _comedi.TRIG_FOLLOW
TRIG_TIME = _comedi.TRIG_TIME
TRIG_TIMER = _comedi.TRIG_TIMER
TRIG_COUNT = _comedi.TRIG_COUNT
TRIG_EXT = _comedi.TRIG_EXT
TRIG_INT = _comedi.TRIG_INT
TRIG_OTHER = _comedi.TRIG_OTHER
SDF_BUSY = _comedi.SDF_BUSY
SDF_BUSY_OWNER = _comedi.SDF_BUSY_OWNER
SDF_LOCKED = _comedi.SDF_LOCKED
SDF_LOCK_OWNER = _comedi.SDF_LOCK_OWNER
SDF_MAXDATA = _comedi.SDF_MAXDATA
SDF_FLAGS = _comedi.SDF_FLAGS
SDF_RANGETYPE = _comedi.SDF_RANGETYPE
SDF_MODE0 = _comedi.SDF_MODE0
SDF_MODE1 = _comedi.SDF_MODE1
SDF_MODE2 = _comedi.SDF_MODE2
SDF_MODE3 = _comedi.SDF_MODE3
SDF_MODE4 = _comedi.SDF_MODE4
SDF_CMD = _comedi.SDF_CMD
SDF_READABLE = _comedi.SDF_READABLE
SDF_WRITABLE = _comedi.SDF_WRITABLE
SDF_WRITEABLE = _comedi.SDF_WRITEABLE
SDF_INTERNAL = _comedi.SDF_INTERNAL
SDF_RT = _comedi.SDF_RT
SDF_GROUND = _comedi.SDF_GROUND
SDF_COMMON = _comedi.SDF_COMMON
SDF_DIFF = _comedi.SDF_DIFF
SDF_OTHER = _comedi.SDF_OTHER
SDF_DITHER = _comedi.SDF_DITHER
SDF_DEGLITCH = _comedi.SDF_DEGLITCH
SDF_MMAP = _comedi.SDF_MMAP
SDF_RUNNING = _comedi.SDF_RUNNING
SDF_LSAMPL = _comedi.SDF_LSAMPL
SDF_PACKED = _comedi.SDF_PACKED
COMEDI_SUBD_UNUSED = _comedi.COMEDI_SUBD_UNUSED
COMEDI_SUBD_AI = _comedi.COMEDI_SUBD_AI
COMEDI_SUBD_AO = _comedi.COMEDI_SUBD_AO
COMEDI_SUBD_DI = _comedi.COMEDI_SUBD_DI
COMEDI_SUBD_DO = _comedi.COMEDI_SUBD_DO
COMEDI_SUBD_DIO = _comedi.COMEDI_SUBD_DIO
COMEDI_SUBD_COUNTER = _comedi.COMEDI_SUBD_COUNTER
COMEDI_SUBD_TIMER = _comedi.COMEDI_SUBD_TIMER
COMEDI_SUBD_MEMORY = _comedi.COMEDI_SUBD_MEMORY
COMEDI_SUBD_CALIB = _comedi.COMEDI_SUBD_CALIB
COMEDI_SUBD_PROC = _comedi.COMEDI_SUBD_PROC
COMEDI_INPUT = _comedi.COMEDI_INPUT
COMEDI_OUTPUT = _comedi.COMEDI_OUTPUT
COMEDI_OPENDRAIN = _comedi.COMEDI_OPENDRAIN
INSN_CONFIG_ANALOG_TRIG = _comedi.INSN_CONFIG_ANALOG_TRIG
INSN_CONFIG_ALT_SOURCE = _comedi.INSN_CONFIG_ALT_SOURCE
CIO = _comedi.CIO
class comedi_trig_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_trig_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_trig_struct, name)
    __swig_setmethods__["subdev"] = _comedi.comedi_trig_struct_subdev_set
    __swig_getmethods__["subdev"] = _comedi.comedi_trig_struct_subdev_get
    if _newclass:subdev = property(_comedi.comedi_trig_struct_subdev_get,_comedi.comedi_trig_struct_subdev_set)
    __swig_setmethods__["mode"] = _comedi.comedi_trig_struct_mode_set
    __swig_getmethods__["mode"] = _comedi.comedi_trig_struct_mode_get
    if _newclass:mode = property(_comedi.comedi_trig_struct_mode_get,_comedi.comedi_trig_struct_mode_set)
    __swig_setmethods__["flags"] = _comedi.comedi_trig_struct_flags_set
    __swig_getmethods__["flags"] = _comedi.comedi_trig_struct_flags_get
    if _newclass:flags = property(_comedi.comedi_trig_struct_flags_get,_comedi.comedi_trig_struct_flags_set)
    __swig_setmethods__["n_chan"] = _comedi.comedi_trig_struct_n_chan_set
    __swig_getmethods__["n_chan"] = _comedi.comedi_trig_struct_n_chan_get
    if _newclass:n_chan = property(_comedi.comedi_trig_struct_n_chan_get,_comedi.comedi_trig_struct_n_chan_set)
    __swig_setmethods__["chanlist"] = _comedi.comedi_trig_struct_chanlist_set
    __swig_getmethods__["chanlist"] = _comedi.comedi_trig_struct_chanlist_get
    if _newclass:chanlist = property(_comedi.comedi_trig_struct_chanlist_get,_comedi.comedi_trig_struct_chanlist_set)
    __swig_setmethods__["data"] = _comedi.comedi_trig_struct_data_set
    __swig_getmethods__["data"] = _comedi.comedi_trig_struct_data_get
    if _newclass:data = property(_comedi.comedi_trig_struct_data_get,_comedi.comedi_trig_struct_data_set)
    __swig_setmethods__["n"] = _comedi.comedi_trig_struct_n_set
    __swig_getmethods__["n"] = _comedi.comedi_trig_struct_n_get
    if _newclass:n = property(_comedi.comedi_trig_struct_n_get,_comedi.comedi_trig_struct_n_set)
    __swig_setmethods__["trigsrc"] = _comedi.comedi_trig_struct_trigsrc_set
    __swig_getmethods__["trigsrc"] = _comedi.comedi_trig_struct_trigsrc_get
    if _newclass:trigsrc = property(_comedi.comedi_trig_struct_trigsrc_get,_comedi.comedi_trig_struct_trigsrc_set)
    __swig_setmethods__["trigvar"] = _comedi.comedi_trig_struct_trigvar_set
    __swig_getmethods__["trigvar"] = _comedi.comedi_trig_struct_trigvar_get
    if _newclass:trigvar = property(_comedi.comedi_trig_struct_trigvar_get,_comedi.comedi_trig_struct_trigvar_set)
    __swig_setmethods__["trigvar1"] = _comedi.comedi_trig_struct_trigvar1_set
    __swig_getmethods__["trigvar1"] = _comedi.comedi_trig_struct_trigvar1_get
    if _newclass:trigvar1 = property(_comedi.comedi_trig_struct_trigvar1_get,_comedi.comedi_trig_struct_trigvar1_set)
    __swig_setmethods__["data_len"] = _comedi.comedi_trig_struct_data_len_set
    __swig_getmethods__["data_len"] = _comedi.comedi_trig_struct_data_len_get
    if _newclass:data_len = property(_comedi.comedi_trig_struct_data_len_get,_comedi.comedi_trig_struct_data_len_set)
    __swig_setmethods__["unused"] = _comedi.comedi_trig_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_trig_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_trig_struct_unused_get,_comedi.comedi_trig_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_trig_struct, 'this', apply(_comedi.new_comedi_trig_struct,args))
        _swig_setattr(self, comedi_trig_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_trig_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_trig_struct instance at %s>" % (self.this,)

class comedi_trig_structPtr(comedi_trig_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_trig_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_trig_struct, 'thisown', 0)
        _swig_setattr(self, comedi_trig_struct,self.__class__,comedi_trig_struct)
_comedi.comedi_trig_struct_swigregister(comedi_trig_structPtr)

class comedi_insn_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_insn_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_insn_struct, name)
    __swig_setmethods__["insn"] = _comedi.comedi_insn_struct_insn_set
    __swig_getmethods__["insn"] = _comedi.comedi_insn_struct_insn_get
    if _newclass:insn = property(_comedi.comedi_insn_struct_insn_get,_comedi.comedi_insn_struct_insn_set)
    __swig_setmethods__["n"] = _comedi.comedi_insn_struct_n_set
    __swig_getmethods__["n"] = _comedi.comedi_insn_struct_n_get
    if _newclass:n = property(_comedi.comedi_insn_struct_n_get,_comedi.comedi_insn_struct_n_set)
    __swig_setmethods__["data"] = _comedi.comedi_insn_struct_data_set
    __swig_getmethods__["data"] = _comedi.comedi_insn_struct_data_get
    if _newclass:data = property(_comedi.comedi_insn_struct_data_get,_comedi.comedi_insn_struct_data_set)
    __swig_setmethods__["subdev"] = _comedi.comedi_insn_struct_subdev_set
    __swig_getmethods__["subdev"] = _comedi.comedi_insn_struct_subdev_get
    if _newclass:subdev = property(_comedi.comedi_insn_struct_subdev_get,_comedi.comedi_insn_struct_subdev_set)
    __swig_setmethods__["chanspec"] = _comedi.comedi_insn_struct_chanspec_set
    __swig_getmethods__["chanspec"] = _comedi.comedi_insn_struct_chanspec_get
    if _newclass:chanspec = property(_comedi.comedi_insn_struct_chanspec_get,_comedi.comedi_insn_struct_chanspec_set)
    __swig_setmethods__["unused"] = _comedi.comedi_insn_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_insn_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_insn_struct_unused_get,_comedi.comedi_insn_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_insn_struct, 'this', apply(_comedi.new_comedi_insn_struct,args))
        _swig_setattr(self, comedi_insn_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_insn_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_insn_struct instance at %s>" % (self.this,)

class comedi_insn_structPtr(comedi_insn_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_insn_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_insn_struct, 'thisown', 0)
        _swig_setattr(self, comedi_insn_struct,self.__class__,comedi_insn_struct)
_comedi.comedi_insn_struct_swigregister(comedi_insn_structPtr)

class comedi_insnlist_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_insnlist_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_insnlist_struct, name)
    __swig_setmethods__["n_insns"] = _comedi.comedi_insnlist_struct_n_insns_set
    __swig_getmethods__["n_insns"] = _comedi.comedi_insnlist_struct_n_insns_get
    if _newclass:n_insns = property(_comedi.comedi_insnlist_struct_n_insns_get,_comedi.comedi_insnlist_struct_n_insns_set)
    __swig_setmethods__["insns"] = _comedi.comedi_insnlist_struct_insns_set
    __swig_getmethods__["insns"] = _comedi.comedi_insnlist_struct_insns_get
    if _newclass:insns = property(_comedi.comedi_insnlist_struct_insns_get,_comedi.comedi_insnlist_struct_insns_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_insnlist_struct, 'this', apply(_comedi.new_comedi_insnlist_struct,args))
        _swig_setattr(self, comedi_insnlist_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_insnlist_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_insnlist_struct instance at %s>" % (self.this,)

class comedi_insnlist_structPtr(comedi_insnlist_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_insnlist_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_insnlist_struct, 'thisown', 0)
        _swig_setattr(self, comedi_insnlist_struct,self.__class__,comedi_insnlist_struct)
_comedi.comedi_insnlist_struct_swigregister(comedi_insnlist_structPtr)

class comedi_cmd_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_cmd_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_cmd_struct, name)
    __swig_setmethods__["subdev"] = _comedi.comedi_cmd_struct_subdev_set
    __swig_getmethods__["subdev"] = _comedi.comedi_cmd_struct_subdev_get
    if _newclass:subdev = property(_comedi.comedi_cmd_struct_subdev_get,_comedi.comedi_cmd_struct_subdev_set)
    __swig_setmethods__["flags"] = _comedi.comedi_cmd_struct_flags_set
    __swig_getmethods__["flags"] = _comedi.comedi_cmd_struct_flags_get
    if _newclass:flags = property(_comedi.comedi_cmd_struct_flags_get,_comedi.comedi_cmd_struct_flags_set)
    __swig_setmethods__["start_src"] = _comedi.comedi_cmd_struct_start_src_set
    __swig_getmethods__["start_src"] = _comedi.comedi_cmd_struct_start_src_get
    if _newclass:start_src = property(_comedi.comedi_cmd_struct_start_src_get,_comedi.comedi_cmd_struct_start_src_set)
    __swig_setmethods__["start_arg"] = _comedi.comedi_cmd_struct_start_arg_set
    __swig_getmethods__["start_arg"] = _comedi.comedi_cmd_struct_start_arg_get
    if _newclass:start_arg = property(_comedi.comedi_cmd_struct_start_arg_get,_comedi.comedi_cmd_struct_start_arg_set)
    __swig_setmethods__["scan_begin_src"] = _comedi.comedi_cmd_struct_scan_begin_src_set
    __swig_getmethods__["scan_begin_src"] = _comedi.comedi_cmd_struct_scan_begin_src_get
    if _newclass:scan_begin_src = property(_comedi.comedi_cmd_struct_scan_begin_src_get,_comedi.comedi_cmd_struct_scan_begin_src_set)
    __swig_setmethods__["scan_begin_arg"] = _comedi.comedi_cmd_struct_scan_begin_arg_set
    __swig_getmethods__["scan_begin_arg"] = _comedi.comedi_cmd_struct_scan_begin_arg_get
    if _newclass:scan_begin_arg = property(_comedi.comedi_cmd_struct_scan_begin_arg_get,_comedi.comedi_cmd_struct_scan_begin_arg_set)
    __swig_setmethods__["convert_src"] = _comedi.comedi_cmd_struct_convert_src_set
    __swig_getmethods__["convert_src"] = _comedi.comedi_cmd_struct_convert_src_get
    if _newclass:convert_src = property(_comedi.comedi_cmd_struct_convert_src_get,_comedi.comedi_cmd_struct_convert_src_set)
    __swig_setmethods__["convert_arg"] = _comedi.comedi_cmd_struct_convert_arg_set
    __swig_getmethods__["convert_arg"] = _comedi.comedi_cmd_struct_convert_arg_get
    if _newclass:convert_arg = property(_comedi.comedi_cmd_struct_convert_arg_get,_comedi.comedi_cmd_struct_convert_arg_set)
    __swig_setmethods__["scan_end_src"] = _comedi.comedi_cmd_struct_scan_end_src_set
    __swig_getmethods__["scan_end_src"] = _comedi.comedi_cmd_struct_scan_end_src_get
    if _newclass:scan_end_src = property(_comedi.comedi_cmd_struct_scan_end_src_get,_comedi.comedi_cmd_struct_scan_end_src_set)
    __swig_setmethods__["scan_end_arg"] = _comedi.comedi_cmd_struct_scan_end_arg_set
    __swig_getmethods__["scan_end_arg"] = _comedi.comedi_cmd_struct_scan_end_arg_get
    if _newclass:scan_end_arg = property(_comedi.comedi_cmd_struct_scan_end_arg_get,_comedi.comedi_cmd_struct_scan_end_arg_set)
    __swig_setmethods__["stop_src"] = _comedi.comedi_cmd_struct_stop_src_set
    __swig_getmethods__["stop_src"] = _comedi.comedi_cmd_struct_stop_src_get
    if _newclass:stop_src = property(_comedi.comedi_cmd_struct_stop_src_get,_comedi.comedi_cmd_struct_stop_src_set)
    __swig_setmethods__["stop_arg"] = _comedi.comedi_cmd_struct_stop_arg_set
    __swig_getmethods__["stop_arg"] = _comedi.comedi_cmd_struct_stop_arg_get
    if _newclass:stop_arg = property(_comedi.comedi_cmd_struct_stop_arg_get,_comedi.comedi_cmd_struct_stop_arg_set)
    __swig_setmethods__["chanlist"] = _comedi.comedi_cmd_struct_chanlist_set
    __swig_getmethods__["chanlist"] = _comedi.comedi_cmd_struct_chanlist_get
    if _newclass:chanlist = property(_comedi.comedi_cmd_struct_chanlist_get,_comedi.comedi_cmd_struct_chanlist_set)
    __swig_setmethods__["chanlist_len"] = _comedi.comedi_cmd_struct_chanlist_len_set
    __swig_getmethods__["chanlist_len"] = _comedi.comedi_cmd_struct_chanlist_len_get
    if _newclass:chanlist_len = property(_comedi.comedi_cmd_struct_chanlist_len_get,_comedi.comedi_cmd_struct_chanlist_len_set)
    __swig_setmethods__["data"] = _comedi.comedi_cmd_struct_data_set
    __swig_getmethods__["data"] = _comedi.comedi_cmd_struct_data_get
    if _newclass:data = property(_comedi.comedi_cmd_struct_data_get,_comedi.comedi_cmd_struct_data_set)
    __swig_setmethods__["data_len"] = _comedi.comedi_cmd_struct_data_len_set
    __swig_getmethods__["data_len"] = _comedi.comedi_cmd_struct_data_len_get
    if _newclass:data_len = property(_comedi.comedi_cmd_struct_data_len_get,_comedi.comedi_cmd_struct_data_len_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_cmd_struct, 'this', apply(_comedi.new_comedi_cmd_struct,args))
        _swig_setattr(self, comedi_cmd_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_cmd_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_cmd_struct instance at %s>" % (self.this,)

class comedi_cmd_structPtr(comedi_cmd_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_cmd_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_cmd_struct, 'thisown', 0)
        _swig_setattr(self, comedi_cmd_struct,self.__class__,comedi_cmd_struct)
_comedi.comedi_cmd_struct_swigregister(comedi_cmd_structPtr)

class comedi_chaninfo_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_chaninfo_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_chaninfo_struct, name)
    __swig_setmethods__["subdev"] = _comedi.comedi_chaninfo_struct_subdev_set
    __swig_getmethods__["subdev"] = _comedi.comedi_chaninfo_struct_subdev_get
    if _newclass:subdev = property(_comedi.comedi_chaninfo_struct_subdev_get,_comedi.comedi_chaninfo_struct_subdev_set)
    __swig_setmethods__["maxdata_list"] = _comedi.comedi_chaninfo_struct_maxdata_list_set
    __swig_getmethods__["maxdata_list"] = _comedi.comedi_chaninfo_struct_maxdata_list_get
    if _newclass:maxdata_list = property(_comedi.comedi_chaninfo_struct_maxdata_list_get,_comedi.comedi_chaninfo_struct_maxdata_list_set)
    __swig_setmethods__["flaglist"] = _comedi.comedi_chaninfo_struct_flaglist_set
    __swig_getmethods__["flaglist"] = _comedi.comedi_chaninfo_struct_flaglist_get
    if _newclass:flaglist = property(_comedi.comedi_chaninfo_struct_flaglist_get,_comedi.comedi_chaninfo_struct_flaglist_set)
    __swig_setmethods__["rangelist"] = _comedi.comedi_chaninfo_struct_rangelist_set
    __swig_getmethods__["rangelist"] = _comedi.comedi_chaninfo_struct_rangelist_get
    if _newclass:rangelist = property(_comedi.comedi_chaninfo_struct_rangelist_get,_comedi.comedi_chaninfo_struct_rangelist_set)
    __swig_setmethods__["unused"] = _comedi.comedi_chaninfo_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_chaninfo_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_chaninfo_struct_unused_get,_comedi.comedi_chaninfo_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_chaninfo_struct, 'this', apply(_comedi.new_comedi_chaninfo_struct,args))
        _swig_setattr(self, comedi_chaninfo_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_chaninfo_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_chaninfo_struct instance at %s>" % (self.this,)

class comedi_chaninfo_structPtr(comedi_chaninfo_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_chaninfo_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_chaninfo_struct, 'thisown', 0)
        _swig_setattr(self, comedi_chaninfo_struct,self.__class__,comedi_chaninfo_struct)
_comedi.comedi_chaninfo_struct_swigregister(comedi_chaninfo_structPtr)

class comedi_rangeinfo_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_rangeinfo_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_rangeinfo_struct, name)
    __swig_setmethods__["range_type"] = _comedi.comedi_rangeinfo_struct_range_type_set
    __swig_getmethods__["range_type"] = _comedi.comedi_rangeinfo_struct_range_type_get
    if _newclass:range_type = property(_comedi.comedi_rangeinfo_struct_range_type_get,_comedi.comedi_rangeinfo_struct_range_type_set)
    __swig_setmethods__["range_ptr"] = _comedi.comedi_rangeinfo_struct_range_ptr_set
    __swig_getmethods__["range_ptr"] = _comedi.comedi_rangeinfo_struct_range_ptr_get
    if _newclass:range_ptr = property(_comedi.comedi_rangeinfo_struct_range_ptr_get,_comedi.comedi_rangeinfo_struct_range_ptr_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_rangeinfo_struct, 'this', apply(_comedi.new_comedi_rangeinfo_struct,args))
        _swig_setattr(self, comedi_rangeinfo_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_rangeinfo_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_rangeinfo_struct instance at %s>" % (self.this,)

class comedi_rangeinfo_structPtr(comedi_rangeinfo_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_rangeinfo_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_rangeinfo_struct, 'thisown', 0)
        _swig_setattr(self, comedi_rangeinfo_struct,self.__class__,comedi_rangeinfo_struct)
_comedi.comedi_rangeinfo_struct_swigregister(comedi_rangeinfo_structPtr)

class comedi_krange_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_krange_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_krange_struct, name)
    __swig_setmethods__["min"] = _comedi.comedi_krange_struct_min_set
    __swig_getmethods__["min"] = _comedi.comedi_krange_struct_min_get
    if _newclass:min = property(_comedi.comedi_krange_struct_min_get,_comedi.comedi_krange_struct_min_set)
    __swig_setmethods__["max"] = _comedi.comedi_krange_struct_max_set
    __swig_getmethods__["max"] = _comedi.comedi_krange_struct_max_get
    if _newclass:max = property(_comedi.comedi_krange_struct_max_get,_comedi.comedi_krange_struct_max_set)
    __swig_setmethods__["flags"] = _comedi.comedi_krange_struct_flags_set
    __swig_getmethods__["flags"] = _comedi.comedi_krange_struct_flags_get
    if _newclass:flags = property(_comedi.comedi_krange_struct_flags_get,_comedi.comedi_krange_struct_flags_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_krange_struct, 'this', apply(_comedi.new_comedi_krange_struct,args))
        _swig_setattr(self, comedi_krange_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_krange_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_krange_struct instance at %s>" % (self.this,)

class comedi_krange_structPtr(comedi_krange_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_krange_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_krange_struct, 'thisown', 0)
        _swig_setattr(self, comedi_krange_struct,self.__class__,comedi_krange_struct)
_comedi.comedi_krange_struct_swigregister(comedi_krange_structPtr)

class comedi_subdinfo_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_subdinfo_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_subdinfo_struct, name)
    __swig_setmethods__["type"] = _comedi.comedi_subdinfo_struct_type_set
    __swig_getmethods__["type"] = _comedi.comedi_subdinfo_struct_type_get
    if _newclass:type = property(_comedi.comedi_subdinfo_struct_type_get,_comedi.comedi_subdinfo_struct_type_set)
    __swig_setmethods__["n_chan"] = _comedi.comedi_subdinfo_struct_n_chan_set
    __swig_getmethods__["n_chan"] = _comedi.comedi_subdinfo_struct_n_chan_get
    if _newclass:n_chan = property(_comedi.comedi_subdinfo_struct_n_chan_get,_comedi.comedi_subdinfo_struct_n_chan_set)
    __swig_setmethods__["subd_flags"] = _comedi.comedi_subdinfo_struct_subd_flags_set
    __swig_getmethods__["subd_flags"] = _comedi.comedi_subdinfo_struct_subd_flags_get
    if _newclass:subd_flags = property(_comedi.comedi_subdinfo_struct_subd_flags_get,_comedi.comedi_subdinfo_struct_subd_flags_set)
    __swig_setmethods__["timer_type"] = _comedi.comedi_subdinfo_struct_timer_type_set
    __swig_getmethods__["timer_type"] = _comedi.comedi_subdinfo_struct_timer_type_get
    if _newclass:timer_type = property(_comedi.comedi_subdinfo_struct_timer_type_get,_comedi.comedi_subdinfo_struct_timer_type_set)
    __swig_setmethods__["len_chanlist"] = _comedi.comedi_subdinfo_struct_len_chanlist_set
    __swig_getmethods__["len_chanlist"] = _comedi.comedi_subdinfo_struct_len_chanlist_get
    if _newclass:len_chanlist = property(_comedi.comedi_subdinfo_struct_len_chanlist_get,_comedi.comedi_subdinfo_struct_len_chanlist_set)
    __swig_setmethods__["maxdata"] = _comedi.comedi_subdinfo_struct_maxdata_set
    __swig_getmethods__["maxdata"] = _comedi.comedi_subdinfo_struct_maxdata_get
    if _newclass:maxdata = property(_comedi.comedi_subdinfo_struct_maxdata_get,_comedi.comedi_subdinfo_struct_maxdata_set)
    __swig_setmethods__["flags"] = _comedi.comedi_subdinfo_struct_flags_set
    __swig_getmethods__["flags"] = _comedi.comedi_subdinfo_struct_flags_get
    if _newclass:flags = property(_comedi.comedi_subdinfo_struct_flags_get,_comedi.comedi_subdinfo_struct_flags_set)
    __swig_setmethods__["range_type"] = _comedi.comedi_subdinfo_struct_range_type_set
    __swig_getmethods__["range_type"] = _comedi.comedi_subdinfo_struct_range_type_get
    if _newclass:range_type = property(_comedi.comedi_subdinfo_struct_range_type_get,_comedi.comedi_subdinfo_struct_range_type_set)
    __swig_setmethods__["unused"] = _comedi.comedi_subdinfo_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_subdinfo_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_subdinfo_struct_unused_get,_comedi.comedi_subdinfo_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_subdinfo_struct, 'this', apply(_comedi.new_comedi_subdinfo_struct,args))
        _swig_setattr(self, comedi_subdinfo_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_subdinfo_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_subdinfo_struct instance at %s>" % (self.this,)

class comedi_subdinfo_structPtr(comedi_subdinfo_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_subdinfo_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_subdinfo_struct, 'thisown', 0)
        _swig_setattr(self, comedi_subdinfo_struct,self.__class__,comedi_subdinfo_struct)
_comedi.comedi_subdinfo_struct_swigregister(comedi_subdinfo_structPtr)

class comedi_devinfo_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_devinfo_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_devinfo_struct, name)
    __swig_setmethods__["version_code"] = _comedi.comedi_devinfo_struct_version_code_set
    __swig_getmethods__["version_code"] = _comedi.comedi_devinfo_struct_version_code_get
    if _newclass:version_code = property(_comedi.comedi_devinfo_struct_version_code_get,_comedi.comedi_devinfo_struct_version_code_set)
    __swig_setmethods__["n_subdevs"] = _comedi.comedi_devinfo_struct_n_subdevs_set
    __swig_getmethods__["n_subdevs"] = _comedi.comedi_devinfo_struct_n_subdevs_get
    if _newclass:n_subdevs = property(_comedi.comedi_devinfo_struct_n_subdevs_get,_comedi.comedi_devinfo_struct_n_subdevs_set)
    __swig_setmethods__["driver_name"] = _comedi.comedi_devinfo_struct_driver_name_set
    __swig_getmethods__["driver_name"] = _comedi.comedi_devinfo_struct_driver_name_get
    if _newclass:driver_name = property(_comedi.comedi_devinfo_struct_driver_name_get,_comedi.comedi_devinfo_struct_driver_name_set)
    __swig_setmethods__["board_name"] = _comedi.comedi_devinfo_struct_board_name_set
    __swig_getmethods__["board_name"] = _comedi.comedi_devinfo_struct_board_name_get
    if _newclass:board_name = property(_comedi.comedi_devinfo_struct_board_name_get,_comedi.comedi_devinfo_struct_board_name_set)
    __swig_setmethods__["read_subdevice"] = _comedi.comedi_devinfo_struct_read_subdevice_set
    __swig_getmethods__["read_subdevice"] = _comedi.comedi_devinfo_struct_read_subdevice_get
    if _newclass:read_subdevice = property(_comedi.comedi_devinfo_struct_read_subdevice_get,_comedi.comedi_devinfo_struct_read_subdevice_set)
    __swig_setmethods__["write_subdevice"] = _comedi.comedi_devinfo_struct_write_subdevice_set
    __swig_getmethods__["write_subdevice"] = _comedi.comedi_devinfo_struct_write_subdevice_get
    if _newclass:write_subdevice = property(_comedi.comedi_devinfo_struct_write_subdevice_get,_comedi.comedi_devinfo_struct_write_subdevice_set)
    __swig_setmethods__["unused"] = _comedi.comedi_devinfo_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_devinfo_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_devinfo_struct_unused_get,_comedi.comedi_devinfo_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_devinfo_struct, 'this', apply(_comedi.new_comedi_devinfo_struct,args))
        _swig_setattr(self, comedi_devinfo_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_devinfo_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_devinfo_struct instance at %s>" % (self.this,)

class comedi_devinfo_structPtr(comedi_devinfo_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_devinfo_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_devinfo_struct, 'thisown', 0)
        _swig_setattr(self, comedi_devinfo_struct,self.__class__,comedi_devinfo_struct)
_comedi.comedi_devinfo_struct_swigregister(comedi_devinfo_structPtr)

class comedi_devconfig_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_devconfig_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_devconfig_struct, name)
    __swig_setmethods__["board_name"] = _comedi.comedi_devconfig_struct_board_name_set
    __swig_getmethods__["board_name"] = _comedi.comedi_devconfig_struct_board_name_get
    if _newclass:board_name = property(_comedi.comedi_devconfig_struct_board_name_get,_comedi.comedi_devconfig_struct_board_name_set)
    __swig_setmethods__["options"] = _comedi.comedi_devconfig_struct_options_set
    __swig_getmethods__["options"] = _comedi.comedi_devconfig_struct_options_get
    if _newclass:options = property(_comedi.comedi_devconfig_struct_options_get,_comedi.comedi_devconfig_struct_options_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_devconfig_struct, 'this', apply(_comedi.new_comedi_devconfig_struct,args))
        _swig_setattr(self, comedi_devconfig_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_devconfig_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_devconfig_struct instance at %s>" % (self.this,)

class comedi_devconfig_structPtr(comedi_devconfig_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_devconfig_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_devconfig_struct, 'thisown', 0)
        _swig_setattr(self, comedi_devconfig_struct,self.__class__,comedi_devconfig_struct)
_comedi.comedi_devconfig_struct_swigregister(comedi_devconfig_structPtr)

class comedi_bufconfig_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_bufconfig_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_bufconfig_struct, name)
    __swig_setmethods__["subdevice"] = _comedi.comedi_bufconfig_struct_subdevice_set
    __swig_getmethods__["subdevice"] = _comedi.comedi_bufconfig_struct_subdevice_get
    if _newclass:subdevice = property(_comedi.comedi_bufconfig_struct_subdevice_get,_comedi.comedi_bufconfig_struct_subdevice_set)
    __swig_setmethods__["flags"] = _comedi.comedi_bufconfig_struct_flags_set
    __swig_getmethods__["flags"] = _comedi.comedi_bufconfig_struct_flags_get
    if _newclass:flags = property(_comedi.comedi_bufconfig_struct_flags_get,_comedi.comedi_bufconfig_struct_flags_set)
    __swig_setmethods__["maximum_size"] = _comedi.comedi_bufconfig_struct_maximum_size_set
    __swig_getmethods__["maximum_size"] = _comedi.comedi_bufconfig_struct_maximum_size_get
    if _newclass:maximum_size = property(_comedi.comedi_bufconfig_struct_maximum_size_get,_comedi.comedi_bufconfig_struct_maximum_size_set)
    __swig_setmethods__["size"] = _comedi.comedi_bufconfig_struct_size_set
    __swig_getmethods__["size"] = _comedi.comedi_bufconfig_struct_size_get
    if _newclass:size = property(_comedi.comedi_bufconfig_struct_size_get,_comedi.comedi_bufconfig_struct_size_set)
    __swig_setmethods__["unused"] = _comedi.comedi_bufconfig_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_bufconfig_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_bufconfig_struct_unused_get,_comedi.comedi_bufconfig_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_bufconfig_struct, 'this', apply(_comedi.new_comedi_bufconfig_struct,args))
        _swig_setattr(self, comedi_bufconfig_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_bufconfig_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_bufconfig_struct instance at %s>" % (self.this,)

class comedi_bufconfig_structPtr(comedi_bufconfig_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_bufconfig_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_bufconfig_struct, 'thisown', 0)
        _swig_setattr(self, comedi_bufconfig_struct,self.__class__,comedi_bufconfig_struct)
_comedi.comedi_bufconfig_struct_swigregister(comedi_bufconfig_structPtr)

class comedi_bufinfo_struct(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_bufinfo_struct, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_bufinfo_struct, name)
    __swig_setmethods__["subdevice"] = _comedi.comedi_bufinfo_struct_subdevice_set
    __swig_getmethods__["subdevice"] = _comedi.comedi_bufinfo_struct_subdevice_get
    if _newclass:subdevice = property(_comedi.comedi_bufinfo_struct_subdevice_get,_comedi.comedi_bufinfo_struct_subdevice_set)
    __swig_setmethods__["bytes_read"] = _comedi.comedi_bufinfo_struct_bytes_read_set
    __swig_getmethods__["bytes_read"] = _comedi.comedi_bufinfo_struct_bytes_read_get
    if _newclass:bytes_read = property(_comedi.comedi_bufinfo_struct_bytes_read_get,_comedi.comedi_bufinfo_struct_bytes_read_set)
    __swig_setmethods__["buf_int_ptr"] = _comedi.comedi_bufinfo_struct_buf_int_ptr_set
    __swig_getmethods__["buf_int_ptr"] = _comedi.comedi_bufinfo_struct_buf_int_ptr_get
    if _newclass:buf_int_ptr = property(_comedi.comedi_bufinfo_struct_buf_int_ptr_get,_comedi.comedi_bufinfo_struct_buf_int_ptr_set)
    __swig_setmethods__["buf_user_ptr"] = _comedi.comedi_bufinfo_struct_buf_user_ptr_set
    __swig_getmethods__["buf_user_ptr"] = _comedi.comedi_bufinfo_struct_buf_user_ptr_get
    if _newclass:buf_user_ptr = property(_comedi.comedi_bufinfo_struct_buf_user_ptr_get,_comedi.comedi_bufinfo_struct_buf_user_ptr_set)
    __swig_setmethods__["buf_int_count"] = _comedi.comedi_bufinfo_struct_buf_int_count_set
    __swig_getmethods__["buf_int_count"] = _comedi.comedi_bufinfo_struct_buf_int_count_get
    if _newclass:buf_int_count = property(_comedi.comedi_bufinfo_struct_buf_int_count_get,_comedi.comedi_bufinfo_struct_buf_int_count_set)
    __swig_setmethods__["buf_user_count"] = _comedi.comedi_bufinfo_struct_buf_user_count_set
    __swig_getmethods__["buf_user_count"] = _comedi.comedi_bufinfo_struct_buf_user_count_get
    if _newclass:buf_user_count = property(_comedi.comedi_bufinfo_struct_buf_user_count_get,_comedi.comedi_bufinfo_struct_buf_user_count_set)
    __swig_setmethods__["unused"] = _comedi.comedi_bufinfo_struct_unused_set
    __swig_getmethods__["unused"] = _comedi.comedi_bufinfo_struct_unused_get
    if _newclass:unused = property(_comedi.comedi_bufinfo_struct_unused_get,_comedi.comedi_bufinfo_struct_unused_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_bufinfo_struct, 'this', apply(_comedi.new_comedi_bufinfo_struct,args))
        _swig_setattr(self, comedi_bufinfo_struct, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_bufinfo_struct):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_bufinfo_struct instance at %s>" % (self.this,)

class comedi_bufinfo_structPtr(comedi_bufinfo_struct):
    def __init__(self,this):
        _swig_setattr(self, comedi_bufinfo_struct, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_bufinfo_struct, 'thisown', 0)
        _swig_setattr(self, comedi_bufinfo_struct,self.__class__,comedi_bufinfo_struct)
_comedi.comedi_bufinfo_struct_swigregister(comedi_bufinfo_structPtr)

RF_EXTERNAL = _comedi.RF_EXTERNAL
UNIT_volt = _comedi.UNIT_volt
UNIT_mA = _comedi.UNIT_mA
UNIT_none = _comedi.UNIT_none
COMEDI_CB_EOS = _comedi.COMEDI_CB_EOS
COMEDI_CB_EOA = _comedi.COMEDI_CB_EOA
COMEDI_CB_BLOCK = _comedi.COMEDI_CB_BLOCK
COMEDI_CB_EOBUF = _comedi.COMEDI_CB_EOBUF
COMEDI_CB_ERROR = _comedi.COMEDI_CB_ERROR
class comedi_range(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_range, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_range, name)
    __swig_setmethods__["min"] = _comedi.comedi_range_min_set
    __swig_getmethods__["min"] = _comedi.comedi_range_min_get
    if _newclass:min = property(_comedi.comedi_range_min_get,_comedi.comedi_range_min_set)
    __swig_setmethods__["max"] = _comedi.comedi_range_max_set
    __swig_getmethods__["max"] = _comedi.comedi_range_max_get
    if _newclass:max = property(_comedi.comedi_range_max_get,_comedi.comedi_range_max_set)
    __swig_setmethods__["unit"] = _comedi.comedi_range_unit_set
    __swig_getmethods__["unit"] = _comedi.comedi_range_unit_get
    if _newclass:unit = property(_comedi.comedi_range_unit_get,_comedi.comedi_range_unit_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_range, 'this', apply(_comedi.new_comedi_range,args))
        _swig_setattr(self, comedi_range, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_range):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_range instance at %s>" % (self.this,)

class comedi_rangePtr(comedi_range):
    def __init__(self,this):
        _swig_setattr(self, comedi_range, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_range, 'thisown', 0)
        _swig_setattr(self, comedi_range,self.__class__,comedi_range)
_comedi.comedi_range_swigregister(comedi_rangePtr)

class comedi_sv_t(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, comedi_sv_t, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, comedi_sv_t, name)
    __swig_setmethods__["dev"] = _comedi.comedi_sv_t_dev_set
    __swig_getmethods__["dev"] = _comedi.comedi_sv_t_dev_get
    if _newclass:dev = property(_comedi.comedi_sv_t_dev_get,_comedi.comedi_sv_t_dev_set)
    __swig_setmethods__["subdevice"] = _comedi.comedi_sv_t_subdevice_set
    __swig_getmethods__["subdevice"] = _comedi.comedi_sv_t_subdevice_get
    if _newclass:subdevice = property(_comedi.comedi_sv_t_subdevice_get,_comedi.comedi_sv_t_subdevice_set)
    __swig_setmethods__["chan"] = _comedi.comedi_sv_t_chan_set
    __swig_getmethods__["chan"] = _comedi.comedi_sv_t_chan_get
    if _newclass:chan = property(_comedi.comedi_sv_t_chan_get,_comedi.comedi_sv_t_chan_set)
    __swig_setmethods__["range"] = _comedi.comedi_sv_t_range_set
    __swig_getmethods__["range"] = _comedi.comedi_sv_t_range_get
    if _newclass:range = property(_comedi.comedi_sv_t_range_get,_comedi.comedi_sv_t_range_set)
    __swig_setmethods__["aref"] = _comedi.comedi_sv_t_aref_set
    __swig_getmethods__["aref"] = _comedi.comedi_sv_t_aref_get
    if _newclass:aref = property(_comedi.comedi_sv_t_aref_get,_comedi.comedi_sv_t_aref_set)
    __swig_setmethods__["n"] = _comedi.comedi_sv_t_n_set
    __swig_getmethods__["n"] = _comedi.comedi_sv_t_n_get
    if _newclass:n = property(_comedi.comedi_sv_t_n_get,_comedi.comedi_sv_t_n_set)
    __swig_setmethods__["maxdata"] = _comedi.comedi_sv_t_maxdata_set
    __swig_getmethods__["maxdata"] = _comedi.comedi_sv_t_maxdata_get
    if _newclass:maxdata = property(_comedi.comedi_sv_t_maxdata_get,_comedi.comedi_sv_t_maxdata_set)
    def __init__(self,*args):
        _swig_setattr(self, comedi_sv_t, 'this', apply(_comedi.new_comedi_sv_t,args))
        _swig_setattr(self, comedi_sv_t, 'thisown', 1)
    def __del__(self, destroy= _comedi.delete_comedi_sv_t):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __repr__(self):
        return "<C comedi_sv_t instance at %s>" % (self.this,)

class comedi_sv_tPtr(comedi_sv_t):
    def __init__(self,this):
        _swig_setattr(self, comedi_sv_t, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, comedi_sv_t, 'thisown', 0)
        _swig_setattr(self, comedi_sv_t,self.__class__,comedi_sv_t)
_comedi.comedi_sv_t_swigregister(comedi_sv_tPtr)

COMEDI_OOR_NUMBER = _comedi.COMEDI_OOR_NUMBER
COMEDI_OOR_NAN = _comedi.COMEDI_OOR_NAN
comedi_open = _comedi.comedi_open

comedi_close = _comedi.comedi_close

comedi_loglevel = _comedi.comedi_loglevel

comedi_perror = _comedi.comedi_perror

comedi_strerror = _comedi.comedi_strerror

comedi_errno = _comedi.comedi_errno

comedi_fileno = _comedi.comedi_fileno

comedi_set_global_oor_behavior = _comedi.comedi_set_global_oor_behavior

comedi_get_n_subdevices = _comedi.comedi_get_n_subdevices

comedi_get_version_code = _comedi.comedi_get_version_code

comedi_get_driver_name = _comedi.comedi_get_driver_name

comedi_get_board_name = _comedi.comedi_get_board_name

comedi_get_read_subdevice = _comedi.comedi_get_read_subdevice

comedi_get_write_subdevice = _comedi.comedi_get_write_subdevice

comedi_get_subdevice_type = _comedi.comedi_get_subdevice_type

comedi_find_subdevice_by_type = _comedi.comedi_find_subdevice_by_type

comedi_get_subdevice_flags = _comedi.comedi_get_subdevice_flags

comedi_get_n_channels = _comedi.comedi_get_n_channels

comedi_range_is_chan_specific = _comedi.comedi_range_is_chan_specific

comedi_maxdata_is_chan_specific = _comedi.comedi_maxdata_is_chan_specific

comedi_get_maxdata = _comedi.comedi_get_maxdata

comedi_get_n_ranges = _comedi.comedi_get_n_ranges

comedi_get_range = _comedi.comedi_get_range

comedi_find_range = _comedi.comedi_find_range

comedi_get_buffer_size = _comedi.comedi_get_buffer_size

comedi_get_max_buffer_size = _comedi.comedi_get_max_buffer_size

comedi_set_buffer_size = _comedi.comedi_set_buffer_size

comedi_do_insnlist = _comedi.comedi_do_insnlist

comedi_do_insn = _comedi.comedi_do_insn

comedi_lock = _comedi.comedi_lock

comedi_unlock = _comedi.comedi_unlock

comedi_to_phys = _comedi.comedi_to_phys

comedi_from_phys = _comedi.comedi_from_phys

comedi_sampl_to_phys = _comedi.comedi_sampl_to_phys

comedi_sampl_from_phys = _comedi.comedi_sampl_from_phys

comedi_data_read = _comedi.comedi_data_read

comedi_data_read_n = _comedi.comedi_data_read_n

comedi_data_read_hint = _comedi.comedi_data_read_hint

comedi_data_read_delayed = _comedi.comedi_data_read_delayed

comedi_data_write = _comedi.comedi_data_write

comedi_dio_config = _comedi.comedi_dio_config

comedi_dio_read = _comedi.comedi_dio_read

comedi_dio_write = _comedi.comedi_dio_write

comedi_dio_bitfield = _comedi.comedi_dio_bitfield

comedi_sv_init = _comedi.comedi_sv_init

comedi_sv_update = _comedi.comedi_sv_update

comedi_sv_measure = _comedi.comedi_sv_measure

comedi_get_cmd_src_mask = _comedi.comedi_get_cmd_src_mask

comedi_get_cmd_generic_timed = _comedi.comedi_get_cmd_generic_timed

comedi_cancel = _comedi.comedi_cancel

comedi_command = _comedi.comedi_command

comedi_command_test = _comedi.comedi_command_test

comedi_poll = _comedi.comedi_poll

comedi_set_max_buffer_size = _comedi.comedi_set_max_buffer_size

comedi_get_buffer_contents = _comedi.comedi_get_buffer_contents

comedi_mark_buffer_read = _comedi.comedi_mark_buffer_read

comedi_get_buffer_offset = _comedi.comedi_get_buffer_offset


