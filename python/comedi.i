/***********************************************************
*           Interface file for wrapping Comedilib                                  
*           author: Bryan Cole  email: bryan.cole@teraview.co.uk  
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
#include "../comedilib-0.7.19/include/comedi.h"
#include "../comedilib-0.7.19/include/comedilib.h"
%}
%include "carrays.i"

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

%array_class(unsigned int, chanlist);

%include "../comedilib-0.7.19/include/comedi.h"
%include "../comedilib-0.7.19/include/comedilib.h"
