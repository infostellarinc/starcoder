/* -*- c++ -*- */
/*
 * Copyright 2018 InfoStellar, Inc.
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
#include "complex_to_msg_c_impl.h"

namespace gr {
namespace starcoder {

complex_to_msg_c::sptr complex_to_msg_c::make(int num_items) {
  return gnuradio::get_initial_sptr(new complex_to_msg_c_impl(num_items));
}

/*
 * The private constructor
 */
complex_to_msg_c_impl::complex_to_msg_c_impl(int num_items)
    : gr::sync_block("complex_to_msg_c",
                     gr::io_signature::make(1, 1, sizeof(gr_complex)),
                     gr::io_signature::make(0, 0, 0)),
      num_items(num_items) {
  // TODO: Send any possible leftover data when closing
  set_output_multiple(num_items);
  message_port_register_out(pmt::mp("out"));
}

/*
 * Our virtual destructor.
 */
complex_to_msg_c_impl::~complex_to_msg_c_impl() {}

int complex_to_msg_c_impl::work(int noutput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items) {
  const gr_complex *in = (const gr_complex *)input_items[0];

  for (int i = 0; i < noutput_items / num_items; i++) {
    message_port_pub(pmt::mp("out"), pmt::mp(in + (i * num_items),
                                             sizeof(gr_complex) * num_items));
  }

  return noutput_items;
}

} /* namespace starcoder */
} /* namespace gr */
