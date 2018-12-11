/* -*- c++ -*- */
/*
 * Copyright 2018 Infostellar, Inc.
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

#include <vector>

#include <gnuradio/io_signature.h>

#include "golay_decoder_impl.h"
#include "golay24.h"

namespace gr {
namespace starcoder {

golay_decoder::sptr golay_decoder::make(int offset, int num_units) {
  return gnuradio::get_initial_sptr(new golay_decoder_impl(offset, num_units));
}

golay_decoder_impl::golay_decoder_impl(int offset, int num_units)
    : gr::block("golay_decoder", gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      offset_(offset),
      num_units_(num_units) {
  message_port_register_in(pmt::mp("in"));
  message_port_register_out(pmt::mp("out"));
  set_msg_handler(pmt::mp("in"),
                  boost::bind(&golay_decoder_impl::msg_handler, this, _1));
}

golay_decoder_impl::~golay_decoder_impl() {}

int golay_decoder_impl::general_work(int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items) {
  return 0;
}

void golay_decoder_impl::msg_handler(pmt::pmt_t pmt_msg) {
  size_t in_size(0);
  const uint8_t *in = pmt::u8vector_elements(pmt::cdr(pmt_msg), in_size);

  std::vector<uint8_t> out;
  std::copy(in, in + offset_, std::back_inserter(out));

  const uint8_t *p = in + offset_;
  for (int i = 0; i < num_units_; i++) {
    uint32_t word = 0;
    for (int j = 0; j < 24; j++) {
      if (j != 0) {
        word = word << 1;
      }
      if (*p != 0) {
        word |= 1;
      }
      p++;
    }
    int num_corrected_bits = decode_golay24(&word);
    if (num_corrected_bits < 0) {
      GR_LOG_DEBUG(d_logger, "Failed to decode a Golay encoded message.");
      return;
    }
    GR_LOG_DEBUG(d_logger, "Decoded a golay encoded message. #corrected bits = "
		 + std::to_string(num_corrected_bits));

    uint32_t mask = 1 << 11;
    for (int j = 0; j < 12; j++) {
      out.push_back(word & mask ? 1 : 0);
      mask = mask >> 1;
    }
  }

  std::copy(in + offset_ + 24 * num_units_, in + in_size,
            std::back_inserter(out));

  message_port_pub(
      pmt::mp("out"),
      pmt::cons(pmt::car(pmt_msg), pmt::init_u8vector(out.size(), out)));
}

} /* namespace starcoder */
} /* namespace gr */
