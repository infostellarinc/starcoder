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

#define CONSECUTIVE_WARNING_LIMIT 10
#define AR2300_SCALE_FACTOR 1E-7

namespace gr {
  namespace starcoder {

    class ar2300_source_impl : public ar2300_source
    {
     private:
      std::unique_ptr<ar2300_receiver> receiver;
      int   timeout_ms;
      int   num_of_consecutive_warns = 0;
      char leftover_[8];
      int num_leftover_ = 0;
      unsigned int num_work_call_ = 0;

      int encode_ar2300(const char* in, int size, gr_complex* out);
      gr_complex parse_sample(const char (&in)[8]) const;
      bool validate_sample(const char (&in)[8]) const;

     public:
      ar2300_source_impl();
      ~ar2300_source_impl();

      // Where all the action really happens
      int work(int n_output_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items) override;
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_AR2300_SOURCE_IMPL_H */
