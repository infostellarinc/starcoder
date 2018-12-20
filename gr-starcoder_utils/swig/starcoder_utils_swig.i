/* -*- c++ -*- */

#define STARCODER_UTILS_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "starcoder_utils_swig_doc.i"

%{
#include "starcoder_utils/folder_source_pdu.h"
%}


%include "starcoder_utils/folder_source_pdu.h"
GR_SWIG_BLOCK_MAGIC2(starcoder_utils, folder_source_pdu);

