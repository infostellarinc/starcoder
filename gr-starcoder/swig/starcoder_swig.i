/* -*- c++ -*- */

#define STARCODER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "starcoder_swig_doc.i"

%{
#include "starcoder/ar2300_source.h"
%}


%include "starcoder/ar2300_source.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, ar2300_source);
