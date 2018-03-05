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

#ifndef INCLUDED_STARCODER_AR2300_SOURCE_IMPL_H
#define INCLUDED_STARCODER_AR2300_SOURCE_IMPL_H

#include <starcoder/ar2300_source.h>
#include "ar2300_receiver.h"

namespace gr {
  namespace starcoder {

    class ar2300_source_impl : public ar2300_source
    {
     private:
      std::unique_ptr<ar2300_receiver> receiver;
      char* buf;
      int   buf_size;
      int   timeout_ms;

      char  sample[8];
      int   sample_index = 0;
      bool  seenIValue = false;

      int encode_ar2300(char* in, int size, gr_complex* out);
      gr_complex parse_sample() const;

     public:
      ar2300_source_impl();
      ~ar2300_source_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_AR2300_SOURCE_IMPL_H */

