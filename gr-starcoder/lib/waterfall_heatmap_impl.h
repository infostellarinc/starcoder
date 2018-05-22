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
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2017, Libre Space Foundation
 * <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_STARCODER_WATERFALL_HEATMAP_IMPL_H
#define INCLUDED_STARCODER_WATERFALL_HEATMAP_IMPL_H

#include <starcoder/waterfall_heatmap.h>
#include <volk/volk.h>
#include <gnuradio/fft/fft.h>

namespace gr {
namespace starcoder {

class waterfall_heatmap_impl : public waterfall_heatmap {
 private:
  /**
   * The different types of operation of the waterfall
   */
  typedef enum {
    WATERFALL_MODE_DECIMATION = 0,  //!< WATERFALL_MODE_DECIMATION Performs just
                                    //a decimation and computes the energy only
    WATERFALL_MODE_MAX_HOLD =
        1,  //!< WATERFALL_MODE_MAX_HOLD compute the max hold energy of all the
            //FFT snapshots between two consecutive pixel rows
  } wf_mode_t;

  const float d_min_energy;
  const float d_max_energy;
  const double d_samp_rate;
  double d_rps;
  const size_t fft_size_;
  wf_mode_t d_mode;
  size_t d_refresh;
  size_t d_fft_cnt;
  size_t d_fft_shift;
  size_t d_samples_cnt;
  fft::fft_complex d_fft;
  float *d_shift_buffer;
  float *d_hold_buffer;
  float *d_tmp_buffer;
  float *d_min_buffer;
  float *d_max_buffer;

  size_t compute_decimation(int8_t *out, const gr_complex *in, size_t n_fft);

  size_t compute_max_hold(int8_t *out, const gr_complex *in, size_t n_fft);

 public:
  waterfall_heatmap_impl(double samp_rate, double center_freq, double pps,
                         size_t fft_size, int mode);
  ~waterfall_heatmap_impl();

  int general_work(int noutput_items, gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_WATERFALL_HEATMAP_IMPL_H */
