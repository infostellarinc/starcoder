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
#include "starcoder/enqueue_message_sink.h"
#include "starcoder/ax25_decoder_bm.h"
#include "starcoder/command_source.h"
#include "starcoder/ax25_encoder_mb.h"
#include "starcoder/noaa_apt_sink.h"
#include "starcoder/meteor_decoder_sink.h"
#include "starcoder/golay_decoder.h"
#include "starcoder/cw_to_symbol.h"
#include "starcoder/morse_decoder.h"
%}


%include "starcoder/ar2300_source.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, ar2300_source);

%include "starcoder/complex_to_msg_c.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, complex_to_msg_c);
%include "starcoder/waterfall_heatmap.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, waterfall_heatmap);
%include "starcoder/waterfall_plotter.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, waterfall_plotter);
%include "starcoder/enqueue_message_sink.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, enqueue_message_sink);
%include "starcoder/ax25_decoder_bm.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, ax25_decoder_bm);
%include "starcoder/command_source.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, command_source);
%include "starcoder/ax25_encoder_mb.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, ax25_encoder_mb);
%include "starcoder/noaa_apt_sink.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, noaa_apt_sink);
%include "starcoder/meteor_decoder_sink.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, meteor_decoder_sink);
%include "starcoder/golay_decoder.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, golay_decoder);
%include "starcoder/cw_to_symbol.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, cw_to_symbol);
%include "starcoder/morse_decoder.h"
GR_SWIG_BLOCK_MAGIC2(starcoder, morse_decoder);
