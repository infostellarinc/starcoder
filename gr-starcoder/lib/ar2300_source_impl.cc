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

#include <gnuradio/io_signature.h>
#include "ar2300_source_impl.h"

namespace gr {
namespace starcoder {

ar2300_source::sptr ar2300_source::make() {
  return gnuradio::get_initial_sptr(new ar2300_source_impl());
}

/*
 * The private constructor
 */
ar2300_source_impl::ar2300_source_impl()
    : gr::sync_block("ar2300_source", gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(1, 1, sizeof(gr_complex))),
      receiver(new ar2300_receiver(10485760)),  // 10MB buffer
      timeout_ms(1000) {
  receiver->start();
}

/*
 * Our virtual destructor.
 */
ar2300_source_impl::~ar2300_source_impl() { receiver->stop(); }

int ar2300_source_impl::work(int n_output_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items) {
  gr_complex *out = (gr_complex *)output_items[0];
  int buf_size = n_output_items * 8;
  char buf[buf_size];

  int ret = receiver->read(buf, buf_size, timeout_ms);
  if (ret < 8) {
    return 0;
  }

  int outSize = encode_ar2300(buf, ret, out);

  // Tell runtime system how many output items we produced.
  return outSize;
}

int ar2300_source_impl::encode_ar2300(const char *in, const int inSize,
                                      gr_complex *out) {
  int offset = 0;
  bool seen_i_value = false;

  for (int i = 1; i < inSize; i += 2) {
    if (in[i] & 0x1) {
      offset = i - 1;
      seen_i_value = true;
      break;
    }
  }

  if (!seen_i_value) {
    return 0;
  }

  char sample[8];
  int sample_index = 0;
  int out_index = 0;

  for (int i = offset; i < inSize; ++i) {
    sample[sample_index++] = in[i];
    if (sample_index == 8) {
      sample_index = 0;
      if (!validate_sample(sample)) {
        GR_LOG_WARN(d_logger, boost::format("Sample may be corrupted"));
        num_of_consecutive_warns++;
        if (num_of_consecutive_warns > CONSECUTIVE_WARNING_LIMIT) {
          GR_LOG_WARN(
              d_logger,
              boost::format("Exceeded the limit of consecutive warnings"));
          throw std::runtime_error("ar2300_source_impl::encode_ar2300()");
        }
        continue;
      }

      out[out_index++] = parse_sample(sample);
      num_of_consecutive_warns = 0;
    }
  }

  return out_index;
}

gr_complex ar2300_source_impl::parse_sample(const char (&in)[8]) const {
  float real = ((in[0] << 24) | (in[1] << 16 & 0xFE0000) |
                (in[2] << 9 & 0x1FE00) | (in[3] << 1 & 0x1FC)) >> 2;
  float imag = ((in[4] << 24) | (in[5] << 16 & 0xFE0000) |
                (in[6] << 9 & 0x1FE00) | (in[7] << 1 & 0x1FC)) >> 2;
  return gr_complex(real * AR2300_SCALE_FACTOR, imag * AR2300_SCALE_FACTOR);
}

bool ar2300_source_impl::validate_sample(const char (&in)[8]) const {
  int i_crc = (in[0] & 0x00) << 24 | (in[1] & 0x01) << 16 |
              (in[2] & 0x00) << 8 | (in[3] & 0x01);
  int q_crc = (in[4] & 0x00) << 24 | (in[5] & 0x01) << 16 |
              (in[6] & 0x00) << 8 | (in[7] & 0x01);
  if ((i_crc == 0x10000) && (q_crc == 0)) {
    return true;
  }
  return false;
}

} /* namespace starcoder */
} /* namespace gr */
