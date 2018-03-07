/* -*- c++ -*- */

#define GRPC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "grpc_swig_doc.i"

%{
#include "grpc/msg_sink.h"
%}


%include "grpc/msg_sink.h"
GR_SWIG_BLOCK_MAGIC2(grpc, msg_sink);
