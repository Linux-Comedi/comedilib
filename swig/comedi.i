/***********************************************************
           Interface file for wrapping Comedilib

    Copyright (C) 2003 Bryan Cole  <bryan.cole@teraview.co.uk>
    Copyright (C) 1998-2002 David A. Schleef <ds@schleef.org>
    Copyright (C) 2003,2004 Frank Mori Hess <fmhess@users.sourceforge.net>
    Copyright (C) 2003 Steven Jenkins <steven.jenkins@ieee.org>
    Copyright (C) 2010-2012 W. Trevor King <wking@drexel.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***********************************************************
*
*     This file was created with Python wrappers in mind but wil
*  probably work for other swig-supported script languages
*
*    to regenerate the wrappers run:
*    swig -python comedi.i
*
***********************************************************/
%module comedi
#define SWIG_USE_OLD_TYPEMAPS
%{
#include "comedilib.h"
#include "comedi.h"
%}
%include "carrays.i"
%include "typemaps.i"

#ifdef SWIGPYTHON
%rename("%(strip:[COMEDI_])s", regextarget=1) "COMEDI_.*";
%rename("%(strip:[comedi_])s", regextarget=1) "comedi_.*";

// These need to be explicitly written as unsigned ints
%rename(CR_FLAGS_MASK) _CR_FLAGS_MASK;
%rename(TRIG_ANY) _TRIG_ANY;
%rename(NI_GPCT_INVERT_CLOCK_SRC_BIT) _NI_GPCT_INVERT_CLOCK_SRC_BIT;
%constant unsigned int _CR_FLAGS_MASK = CR_FLAGS_MASK;
%constant unsigned int _CR_FLAGS_MASK = CR_FLAGS_MASK;
%constant unsigned int _TRIG_ANY = TRIG_ANY;
%constant unsigned int _NI_GPCT_INVERT_CLOCK_SRC_BIT = NI_GPCT_INVERT_CLOCK_SRC_BIT;
#endif

%inline %{
static unsigned int cr_pack(unsigned int chan, unsigned int rng, unsigned int aref){
	return CR_PACK(chan,rng,aref);
}
static unsigned int cr_pack_flags(unsigned int chan, unsigned int rng, unsigned int aref, unsigned int flags){
	return CR_PACK_FLAGS(chan,rng,aref, flags);
}
static unsigned int cr_chan(unsigned int a){
	return CR_CHAN(a);
}
static unsigned int cr_range(unsigned int a){
	return CR_RANGE(a);
}
static unsigned int cr_aref(unsigned int a){
	return CR_AREF(a);
}
%}

// Give swig prototype for each of these macros so they can be properly wrapped.
unsigned int CR_PACK(unsigned int chan, unsigned int rng, unsigned int aref);
unsigned int CR_PACK_FLAGS(unsigned int chan, unsigned int rng, unsigned int aref, unsigned int flags);
unsigned int CR_CHAN(unsigned int chan);
unsigned int CR_RANGE(unsigned int chan);
unsigned int CR_AREF(unsigned int chan);
unsigned int NI_USUAL_PFI_SELECT(unsigned int pfi_channel);
unsigned int NI_USUAL_RTSI_SELECT(unsigned int rtsi_channel);
unsigned int NI_GPCT_SOURCE_PIN_CLOCK_SRC_BITS(unsigned int n);
unsigned int NI_GPCT_RTSI_CLOCK_SRC_BITS(unsigned int n);
unsigned int NI_GPCT_PFI_CLOCK_SRC_BITS(unsigned int n);
unsigned int NI_GPCT_GATE_PIN_GATE_SELECT(unsigned int n);
unsigned int NI_GPCT_RTSI_GATE_SELECT(unsigned int n);
unsigned int NI_GPCT_PFI_GATE_SELECT(unsigned int n);
unsigned int NI_GPCT_UP_DOWN_PIN_GATE_SELECT(unsigned int n);
unsigned int NI_GPCT_PFI_OTHER_SELECT(unsigned int n);
unsigned int NI_MIO_PLL_RTSI_CLOCK(unsigned int rtsi_channel);
unsigned int NI_RTSI_OUTPUT_RTSI_BRD(unsigned int n);
unsigned int NI_PFI_OUTPUT_RTSI(unsigned int rtsi_channel);
unsigned int NI_EXT_PFI(unsigned int pfi_channel);
unsigned int NI_EXT_RTSI(unsigned int rtsi_channel);
unsigned int NI_CDIO_SCAN_BEGIN_SRC_PFI(unsigned int pfi_channel);
unsigned int NI_CDIO_SCAN_BEGIN_SRC_RTSI(unsigned int rtsi_channel);
unsigned int NI_AO_SCAN_BEGIN_SRC_PFI(unsigned int pfi_channel);
unsigned int NI_AO_SCAN_BEGIN_SRC_RTSI(unsigned int rtsi_channel);

#ifdef SWIGRUBY
%typemap(argout) comedi_cmd *INOUT(VALUE info) {
    $result = output_helper($result, $arg);
};
#endif

%include "comedi.h"
%include "comedilib.h"

%array_class(unsigned int, chanlist);
%array_class(sampl_t, sampl_array);
%array_class(lsampl_t, lsampl_array);
%array_class(comedi_insn, insn_array);

%insert("python") %{
# Trick to allow accessing functions and constants with their original C name
import sys

class __Wrapper(object):
    def __init__(self, wrapped):
        self.wrapped = wrapped

    def __getattr__(self, name):
        try:
            return getattr(self.wrapped, name)
        except AttributeError:
            if name.startswith("comedi_") or name.startswith("COMEDI_"):
                return getattr(self.wrapped, name[7:])
            else:
                raise

sys.modules[__name__] = __Wrapper(sys.modules[__name__])
%}

