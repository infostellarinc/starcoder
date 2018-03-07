/* -*- c++ -*- */

#define GRGRPC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "grgrpc_swig_doc.i"

%{
#include "grgrpc/msg_sink.h"
%}


%include "grgrpc/msg_sink.h"
GR_SWIG_BLOCK_MAGIC2(grgrpc, msg_sink);
