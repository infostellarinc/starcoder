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

#ifndef INCLUDED_STARCODER_WATERFALL_HEATMAP_H
#define INCLUDED_STARCODER_WATERFALL_HEATMAP_H

#include <starcoder/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace starcoder {

/*!
 * \brief This block computes the waterfall of the incoming signal
 * and outputs the dB quantized in the range [-128, 127], using a single
 * byte for each carrier.
 * \ingroup starcoder
 *
 */
class STARCODER_API waterfall_heatmap : virtual public gr::block {
 public:
  typedef boost::shared_ptr<waterfall_heatmap> sptr;

  /**
   * This block computes the waterfall of the incoming signal
   * and propagates the result for plotting.
   *
   *
   * @param samp_rate the sampling rate
   * @param center_freq the observation center frequency. Used only for
   * plotting reasons. For a normalized frequency x-axis set it to 0.
   * @param rps rows per second
   * @param fft_size FFT size
   * @param mode the mode that the waterfall.
   * - 0: Simple decimation
   * - 1: Max hold
   * - 2: Mean energy
   *
   * @return shared pointer to the object
   */
  static sptr make(double samp_rate, double center_freq, double rps,
                   size_t fft_size, int mode = 0);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_WATERFALL_HEATMAP_H */
