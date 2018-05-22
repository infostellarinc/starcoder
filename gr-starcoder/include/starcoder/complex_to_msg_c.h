/* -*- c++ -*- */
/*
 * Copyright 2018 InfoStellar, Inc.
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

#ifndef INCLUDED_STARCODER_COMPLEX_TO_MSG_C_H
#define INCLUDED_STARCODER_COMPLEX_TO_MSG_C_H

#include <starcoder/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace starcoder {

/*!
 * This block takes in a stream of complex floats and packs them together
 * into a PMT blob containing `num_items` items.
 *
 */
class STARCODER_API complex_to_msg_c : virtual public gr::sync_block {
 public:
  typedef boost::shared_ptr<complex_to_msg_c> sptr;

  /*!
   * \brief Return a shared_ptr to a new instance of
   * starcoder::complex_to_msg_c.
   *
   * To avoid accidental use of raw pointers, starcoder::complex_to_msg_c's
   * constructor is in a private implementation
   * class. starcoder::complex_to_msg_c::make is the public interface for
   * creating new instances.
   */
  static sptr make(int num_items);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_COMPLEX_TO_MSG_C_H */
