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

#ifndef INCLUDED_STARCODER_METEOR_DECODER_SINK_H
#define INCLUDED_STARCODER_METEOR_DECODER_SINK_H

#include <starcoder/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace starcoder {

/*!
 * \ingroup starcoder
 *
 */
class STARCODER_API meteor_decoder_sink : virtual public gr::sync_block {
 public:
  typedef boost::shared_ptr<meteor_decoder_sink> sptr;

  static sptr make(const std::string &filename_png);
  virtual void register_starcoder_queue(uint64_t ptr) = 0;
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_METEOR_DECODER_SINK_H */
