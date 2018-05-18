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

#ifndef INCLUDED_STARCODER_COMMAND_SOURCE_IMPL_H
#define INCLUDED_STARCODER_COMMAND_SOURCE_IMPL_H

#include <starcoder/command_source.h>
#include <boost/thread/thread.hpp>
#include "string_queue.h"

namespace gr {
  namespace starcoder {

    class command_source_impl : public command_source
    {
     private:
      boost::thread *thread_;
      const pmt::pmt_t port_;
      string_queue queue_;

      void readloop();

     public:
      bool finished_;

      command_source_impl();
      ~command_source_impl();

      bool start();
      bool stop();

      void push(const std::string &message);
      virtual uint64_t get_queue_ptr();
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_COMMAND_SOURCE_IMPL_H */

