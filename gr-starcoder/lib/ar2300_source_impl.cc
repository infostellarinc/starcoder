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

    ar2300_source::sptr
    ar2300_source::make()
    {
      return gnuradio::get_initial_sptr
        (new ar2300_source_impl());
    }

    /*
     * The private constructor
     */
    ar2300_source_impl::ar2300_source_impl()
      : gr::sync_block("ar2300_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        receiver(new ar2300_receiver())
    {
      buf_size = 10240;
      timeout_ms = 1000;
      buf = new char[buf_size];
      receiver->initialize();
    }

    /*
     * Our virtual destructor.
     */
    ar2300_source_impl::~ar2300_source_impl()
    {
      receiver->stop();
      if (buf != NULL) {
         delete[](buf);
      }
    }

    int
    ar2300_source_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      gr_complex *out = (gr_complex *) output_items[0];

      int ret = receiver->read(buf, buf_size, timeout_ms);
      if (ret == 0) {
        return noutput_items;
      }

      int outSize = encode_ar2300(buf, ret, out);

      // Tell runtime system how many output items we produced.
      return outSize;
    }

    int
    ar2300_source_impl::encode_ar2300(char* in,
                              const int inSize,
                              gr_complex* out)
    {
      int out_index = 0;
      for (int i = 0; i < inSize; ++i) {
        sample[sample_index++] = in[i];

        if (sample_index == 8) {
          sample_index = 0;
          gr_complex value = parse_sample();
          out[out_index++] = value;
        }
      }

      return out_index;
    }

    gr_complex
    ar2300_source_impl::parse_sample() const
    {
      float real = ((sample[0] << 24) | (sample[1] << 16 & 16646144) |
                                  (sample[2] << 9 & 130560) | (sample[3] << 1 & 508)) >>
                                 2;
      float imag = ((sample[4] << 24) | (sample[5] << 16 & 16646144) |
                                  (sample[6] << 9 & 130560) | (sample[7] << 1 & 508)) >>
                                 2;
      return gr_complex(real, imag);
    }

  } /* namespace starcoder */
} /* namespace gr */

