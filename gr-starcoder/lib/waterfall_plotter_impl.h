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

#ifndef INCLUDED_STARCODER_WATERFALL_PLOTTER_IMPL_H
#define INCLUDED_STARCODER_WATERFALL_PLOTTER_IMPL_H

#include <list>
#include <starcoder/waterfall_plotter.h>

namespace gr {
  namespace starcoder {
    struct item {
      size_t size;
      char *arr;
    };

    class waterfall_plotter_impl : public waterfall_plotter
    {
     private:
      std::list<item> list_of_arrays_;
      int total_size_;
      double samp_rate_;
      double center_freq_;
      int rps_;
      char* filename_;
      size_t fft_size_;
      void init_numpy_array();

     public:
      waterfall_plotter_impl(double samp_rate, double center_freq,
                             int rps, size_t fft_size, char* filename);
      ~waterfall_plotter_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      virtual bool stop();
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_WATERFALL_PLOTTER_IMPL_H */

