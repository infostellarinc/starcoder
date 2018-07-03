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

#ifndef INCLUDED_STARCODER_METEOR_DECODER_SINK_IMPL_H
#define INCLUDED_STARCODER_METEOR_DECODER_SINK_IMPL_H

#include <starcoder/meteor_decoder_sink.h>
#include <vector>
#include <string_queue.h>

namespace gr {
namespace starcoder {
struct item {
  size_t size;
  const uint8_t *partial_stream;
};

class meteor_decoder_sink_impl : public meteor_decoder_sink {
 private:
  std::string construct_filename(const std::string &original, int apid);

  std::vector<item> items_;
  int total_size_;
  string_queue *string_queue_;
  std::string filename_;

 public:
  meteor_decoder_sink_impl(const std::string &filename_png);
  ~meteor_decoder_sink_impl();

  // Where all the action really happens
  int work(int noutput_items, gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

  bool stop();
  void register_starcoder_queue(uint64_t ptr);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_METEOR_DECODER_SINK_IMPL_H */
