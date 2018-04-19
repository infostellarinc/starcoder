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
 *  Copyright (C) 2017, Libre Space Foundation <http://librespacefoundation.org/>
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


#ifndef INCLUDED_STARCODER_WATERFALL_PLOTTER_H
#define INCLUDED_STARCODER_WATERFALL_PLOTTER_H

#include <starcoder/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace starcoder {

    /*!
     * \brief <+description of block+>
     * \ingroup starcoder
     *
     */
    class STARCODER_API waterfall_plotter : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<waterfall_plotter> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of starcoder::waterfall_plotter.
       *
       * To avoid accidental use of raw pointers, starcoder::waterfall_plotter's
       * constructor is in a private implementation
       * class. starcoder::waterfall_plotter::make is the public interface for
       * creating new instances.
       */
      static sptr
      make(double samp_rate, double center_freq,
           int rps, size_t fft_size, char* filename);
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_WATERFALL_PLOTTER_H */

