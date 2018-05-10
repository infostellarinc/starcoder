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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "waterfall_heatmap_impl.h"

namespace gr {
namespace starcoder {

waterfall_heatmap::sptr waterfall_heatmap::make(double samp_rate,
                                                double center_freq, double rps,
                                                size_t fft_size, int mode) {
  return gnuradio::get_initial_sptr(
      new waterfall_heatmap_impl(samp_rate, center_freq, rps, fft_size, mode));
}

/*
 * The private constructor
 */
waterfall_heatmap_impl::waterfall_heatmap_impl(double samp_rate,
                                               double center_freq, double rps,
                                               size_t fft_size, int mode)
    : gr::block("waterfall_heatmap",
                gr::io_signature::make(1, 1, fft_size * sizeof(gr_complex)),
                gr::io_signature::make(1, 1, fft_size * sizeof(int8_t))),
      d_min_energy(-110.0f),
      d_max_energy(-72.0f),
      d_samp_rate(samp_rate),
      d_rps(rps),
      fft_size_(fft_size),
      d_mode((wf_mode_t) mode),
      d_refresh((d_samp_rate / fft_size) / rps),
      d_fft_cnt(0),
      d_fft_shift((size_t)(ceil(fft_size / 2.0))),
      d_samples_cnt(0),
      d_fft(fft_size) {
  float r = 0.0;
  const int alignment_multiple =
      volk_get_alignment() / (fft_size * sizeof(gr_complex));
  set_alignment(std::max(1, alignment_multiple));

  d_shift_buffer =
      (float *)volk_malloc(fft_size * sizeof(float), volk_get_alignment());
  if (!d_shift_buffer) {
    throw std::runtime_error("Could not allocate aligned memory");
  }

  d_hold_buffer =
      (float *)volk_malloc(fft_size * sizeof(gr_complex), volk_get_alignment());
  if (!d_hold_buffer) {
    throw std::runtime_error("Could not allocate aligned memory");
  }
  memset(d_hold_buffer, 0, fft_size * sizeof(gr_complex));

  d_tmp_buffer =
      (float *)volk_malloc(fft_size * sizeof(float), volk_get_alignment());
  if (!d_tmp_buffer) {
    throw std::runtime_error("Could not allocate aligned memory");
  }

  d_min_buffer =
      (float *)volk_malloc(fft_size * sizeof(float), volk_get_alignment());
  d_max_buffer =
      (float *)volk_malloc(fft_size * sizeof(float), volk_get_alignment());

  if (!d_min_buffer || !d_max_buffer) {
    throw std::runtime_error("Could not allocate aligned memory");
  }

  for (size_t i = 0; i < fft_size; i++) {
    d_min_buffer[i] = d_min_energy;
    d_max_buffer[i] = d_max_energy;
  }
}

/*
 * Our virtual destructor.
 */
waterfall_heatmap_impl::~waterfall_heatmap_impl() {
  volk_free(d_shift_buffer);
  volk_free(d_hold_buffer);
  volk_free(d_tmp_buffer);
  volk_free(d_min_buffer);
  volk_free(d_max_buffer);
}

int waterfall_heatmap_impl::general_work(int noutput_items,
                                         gr_vector_int &ninput_items,
                                         gr_vector_const_void_star &input_items,
                                         gr_vector_void_star &output_items) {
  size_t produced;
  const gr_complex *in = (const gr_complex *)input_items[0];
  int8_t *out = (int8_t *)output_items[0];

  size_t n_fft = std::min(ninput_items[0], noutput_items);

  switch (d_mode) {
    case WATERFALL_MODE_DECIMATION:
      produced = compute_decimation(out, in, n_fft);
      break;
    case WATERFALL_MODE_MAX_HOLD:
      produced = compute_max_hold(out, in, n_fft);
      break;
    default:
      throw std::runtime_error("Wrong waterfall mode");
      return -1;
  }

  consume_each(n_fft);
  return produced;
}

size_t waterfall_heatmap_impl::compute_decimation(int8_t *out,
                                                  const gr_complex *in,
                                                  size_t n_fft) {
  size_t i;
  size_t produced = 0;
  float t;
  gr_complex *fft_in;
  for (i = 0; i < n_fft; i++) {
    d_fft_cnt++;
    if (d_fft_cnt > d_refresh) {
      fft_in = d_fft.get_inbuf();
      memcpy(fft_in, in + i * fft_size_, fft_size_ * sizeof(gr_complex));
      d_fft.execute();

      /* Compute the energy in dB */
      volk_32fc_s32f_x2_power_spectral_density_32f(
          d_shift_buffer, d_fft.get_outbuf(), (float) fft_size_, 1.0,
          fft_size_);
      /* Perform FFT shift */
      memcpy(d_hold_buffer, d_shift_buffer + d_fft_shift,
             sizeof(float) * (fft_size_ - d_fft_shift));
      memcpy(&d_hold_buffer[fft_size_ - d_fft_shift], d_shift_buffer,
             sizeof(float) * d_fft_shift);

      /* Clamp the energy to the [min, max] range */
      volk_32f_x2_max_32f(d_hold_buffer, d_hold_buffer, d_min_buffer,
                          fft_size_);
      volk_32f_x2_min_32f(d_hold_buffer, d_hold_buffer, d_max_buffer,
                          fft_size_);
      volk_32f_s32f_convert_8i(out + produced * fft_size_, d_hold_buffer, 1.0,
                               fft_size_);
      produced++;
      d_fft_cnt = 0;
    }
    d_samples_cnt += fft_size_;
  }
  return produced;
}

size_t waterfall_heatmap_impl::compute_max_hold(int8_t *out,
                                                const gr_complex *in,
                                                size_t n_fft) {
  size_t i;
  size_t produced = 0;
  size_t j;
  float t;
  gr_complex *fft_in;
  for (i = 0; i < n_fft; i++) {
    fft_in = d_fft.get_inbuf();
    memcpy(fft_in, in + i * fft_size_, fft_size_ * sizeof(gr_complex));
    d_fft.execute();

    /* Compute the mag^2 */
    volk_32fc_magnitude_squared_32f(d_tmp_buffer, d_fft.get_outbuf(),
                                    fft_size_);

    /* Normalization factor */
    volk_32f_s32f_multiply_32f(d_tmp_buffer, d_tmp_buffer,
                               1.0 / (fft_size_ * fft_size_), fft_size_);

    /* Max hold */
    volk_32f_x2_max_32f(d_hold_buffer, d_hold_buffer, d_tmp_buffer, fft_size_);
    d_fft_cnt++;
    if (d_fft_cnt > d_refresh) {
      /* Perform FFT shift */
      memcpy(d_shift_buffer, d_hold_buffer + d_fft_shift,
             sizeof(float) * (fft_size_ - d_fft_shift));
      memcpy(&d_shift_buffer[fft_size_ - d_fft_shift], d_hold_buffer,
             sizeof(float) * d_fft_shift);

      /* Compute the energy in dB */
      for (j = 0; j < fft_size_; j++) {
        d_hold_buffer[j] = 10.0 * log10f(d_shift_buffer[j] + 1.0e-20);
      }

      /* Clamp the energy to the [min, max] range */
      volk_32f_x2_max_32f(d_hold_buffer, d_hold_buffer, d_min_buffer,
                          fft_size_);
      volk_32f_x2_min_32f(d_hold_buffer, d_hold_buffer, d_max_buffer,
                          fft_size_);

      /* Reset */
      d_fft_cnt = 0;
      volk_32f_s32f_convert_8i(out + produced * fft_size_, d_hold_buffer, 1.0,
                               fft_size_);
      produced++;
      memset(d_hold_buffer, 0, fft_size_ * sizeof(float));
    }
    d_samples_cnt += fft_size_;
  }
  return produced;
}

} /* namespace starcoder */
} /* namespace gr */
