/* -*- c++ -*- */

#define STARCODER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "starcoder_swig_doc.i"

%{
#include "starcoder/ar2300_source.h"
#include "starcoder/complex_to_msg_c.h"
#include "starcoder/waterfall_heatmap.h"
#include "starcoder/waterfall_plotter.h"
%}


%include "starcoder/ar2300_source.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, ar2300_source);

%include "starcoder/complex_to_msg_c.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, complex_to_msg_c);
%include "starcoder/waterfall_heatmap.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, waterfall_heatmap);
%include "starcoder/waterfall_plotter.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, waterfall_plotter);
