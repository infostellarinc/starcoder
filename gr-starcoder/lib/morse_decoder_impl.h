/* -*- c++ -*- */
/* 
 * Copyright 2019 Infostellar, Inc.
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
 *  Copyright (C) 2016, Libre Space Foundation <http://librespacefoundation.org/>
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

#ifndef INCLUDED_STARCODER_MORSE_DECODER_IMPL_H
#define INCLUDED_STARCODER_MORSE_DECODER_IMPL_H

#include <starcoder/morse_decoder.h>
#include <starcoder/morse_tree.h>

namespace gr {
  namespace starcoder {

    class morse_decoder_impl : public morse_decoder
    {
     private:
      morse_tree d_morse_tree;
      size_t d_min_frame_len;

      void
      symbol_msg_handler(pmt::pmt_t msg);

     public:
      morse_decoder_impl(char unrecognized_char, size_t min_frame_len);

    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_MORSE_DECODER_IMPL_H */

