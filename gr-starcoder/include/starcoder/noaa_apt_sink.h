/* -*- c++ -*- */
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

#ifndef INCLUDED_STARCODER_NOAA_APT_SINK_H
#define INCLUDED_STARCODER_NOAA_APT_SINK_H

#include <starcoder/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace starcoder {

/*!
 * Sink block for NOAA satellites
 * \ingroup starcoder
 *
 */
class STARCODER_API noaa_apt_sink : virtual public gr::sync_block {
 public:
  typedef boost::shared_ptr<noaa_apt_sink> sptr;

  /*!
   * Accepts a stream of floats in the range [0,1] which
   * correspond to one sample per symbol (pixel) and
   * outputs a grayscale PNG image. This block performs normalization on the
   * input
   * float values based on the max and min values observed in the stream.
   * Adding to that, the user has the option to synchronize to the first of the
   * two training sequences used by the NOAA APT protocol so that
   * the two images are displayed one next to the other. The user
   * can also select to rotate the image 180 degrees in case the captured one
   * is upside down.
   *
   *
   * @param filename_png the base filename of the output PNG file(s)
   * @param width the width of the image in the APT transmission
   * @param height the height of the image in the APT transmission
   * @param sync user option for synchronizing to the first of the
   * two training sequences
   * @param flip user option to rotate the image(s) 180 degrees
   *
   */
  static sptr make(const char *filename_png, size_t width, size_t height,
                   bool sync, bool flip);
  virtual void register_starcoder_queue(uint64_t ptr) = 0;
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_NOAA_APT_SINK_H */
