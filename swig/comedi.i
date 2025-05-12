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
%{
#include "comedilib.h"
#include "comedi.h"
%}
%include "carrays.i"
%include "typemaps.i"

// Uncomment to finalize the removal of the comedi_/COMEDI_ prefix and break backwards
// compatibility.
//#define SWIGPYTHONONLYSHORT

#ifdef SWIGPYTHON
// These need to be explicitly written as unsigned ints
%rename(CR_FLAGS_MASK) _CR_FLAGS_MASK;
%rename(CR_INVERT) _CR_INVERT;
%rename(TRIG_ANY) _TRIG_ANY;
%rename(NI_GPCT_INVERT_CLOCK_SRC_BIT) _NI_GPCT_INVERT_CLOCK_SRC_BIT;
%constant unsigned int _CR_FLAGS_MASK = CR_FLAGS_MASK;
%constant unsigned int _CR_INVERT = CR_INVERT;
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
unsigned int NI_PFI(int channel);
unsigned int TRIGGER_LINE(int channel);
unsigned int NI_RTSI_BRD(int channel);
unsigned int NI_CtrSource(int channel);
unsigned int NI_CtrGate(int channel);
unsigned int NI_CtrAux(int channel);
unsigned int NI_CtrA(int channel);
unsigned int NI_CtrB(int channel);
unsigned int NI_CtrZ(int channel);
unsigned int NI_CtrArmStartTrigger(int channel);
unsigned int NI_CtrInternalOutput(int channel);
unsigned int NI_CtrOut(int channel);

#ifdef SWIGRUBY
%typemap(argout) comedi_cmd *INOUT(VALUE info) {
%#if SWIG_VERSION >= 0x040300
    $result = SWIG_Ruby_AppendOutput($result, $arg, $isvoid);
%#else
    $result = SWIG_Ruby_AppendOutput($result, $arg);
%#endif
};

// Ignore invalid names for constants
%ignore _NI_NAMES_MAX_PLUS_1;
#endif

%include "comedi.h"
%include "comedilib.h"

%array_class(unsigned int, chanlist);
%array_class(sampl_t, sampl_array);
%array_class(lsampl_t, lsampl_array);
%array_class(comedi_insn, insn_array);

#ifdef SWIGPYTHON
#ifdef SWIGPYTHONONLYSHORT
%insert("python") %{
  delete_comedi_prefix = True
  %}
#else
%insert("python") %{
  delete_comedi_prefix = False
  %}
#endif

%insert("python") %{
# Add entries in module dictionary without comedi_/COMEDI_ prefix
import re
for k,v in globals().copy().items():
  if re.match('^comedi_', k, flags=re.IGNORECASE):
    globals()[k[7:]] = v
    if delete_comedi_prefix:
        globals().pop(k) # Break backwards compatibility
del re, k, v, delete_comedi_prefix
%}
#endif

