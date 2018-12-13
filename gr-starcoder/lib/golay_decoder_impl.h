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

#ifndef INCLUDED_STARCODER_GOLAY_DECODER_IMPL_H
#define INCLUDED_STARCODER_GOLAY_DECODER_IMPL_H

#include <starcoder/golay_decoder.h>

namespace gr {
namespace starcoder {

class golay_decoder_impl : public golay_decoder {
 public:
  golay_decoder_impl(int offset, int num_units);
  ~golay_decoder_impl();

  int general_work(int noutput_items, gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items);

 private:
  void msg_handler(pmt::pmt_t pmt_msg);

  const int offset_;
  const int num_units_;
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_GOLAY_DECODER_IMPL_H */
