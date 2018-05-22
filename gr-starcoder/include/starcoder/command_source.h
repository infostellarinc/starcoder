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

#ifndef INCLUDED_STARCODER_COMMAND_SOURCE_H
#define INCLUDED_STARCODER_COMMAND_SOURCE_H

#include <starcoder/api.h>
#include <gnuradio/block.h>

namespace gr {
namespace starcoder {

/*!
 * This block is used directly by Starcoder to send PMTs to any
 * blocks with a message input.
 *
 */
class STARCODER_API command_source : virtual public gr::block {
 public:
  typedef boost::shared_ptr<command_source> sptr;

  static sptr make();
  virtual void push(const std::string &message) = 0;
  virtual uint64_t get_starcoder_queue_ptr() = 0;
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_STARCODER_COMMAND_SOURCE_H */
