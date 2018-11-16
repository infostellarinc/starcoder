/* -*- c++ -*- */
/* 
 * Copyright 2018 Infostellar.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "golay_decoder_impl.h"

namespace gr {
  namespace starcoder {

    golay_decoder::sptr
    golay_decoder::make(int offset, int num_units)
    {
      return gnuradio::get_initial_sptr
        (new golay_decoder_impl(offset, num_units));
    }

    golay_decoder_impl::golay_decoder_impl(int offset, int num_units)
      : gr::block("golay_decoder",
		  gr::io_signature::make(0, 0, 0),
                  gr::io_signature::make(0, 0, 0))
    {
      message_port_register_in(pmt::mp("in"));
      message_port_register_out(pmt::mp("out"));
      set_msg_handler(pmt::mp("in"), boost::bind(&golay_decoder_impl::msg_handler, this, _1\
));
    }

    golay_decoder_impl::~golay_decoder_impl()
    {
    }

    void
    golay_decoder_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    golay_decoder_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      return 0;
    }

    void golay_decoder_impl::msg_handler(pmt::pmt_t pmt_msg) {}


  } /* namespace starcoder */
} /* namespace gr */

