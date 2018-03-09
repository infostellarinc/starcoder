/* -*- c++ -*- */

#define GRPCALL_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "grpcall_swig_doc.i"

%{
#include "grpcall/msg_sink.h"
%}


%include "grpcall/msg_sink.h"
GR_SWIG_BLOCK_MAGIC2(grpcall, msg_sink);

