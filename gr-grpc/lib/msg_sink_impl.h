/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_GRPC_MSG_SINK_IMPL_H
#define INCLUDED_GRPC_MSG_SINK_IMPL_H

#include <grpc/msg_sink.h>

namespace gr {
  namespace grpc {

    class msg_sink_impl : public msg_sink
    {
     private:
      // Nothing to declare in this block.

     public:
      msg_sink_impl(char *address);
      ~msg_sink_impl();

      void handler(pmt::pmt_t msg);
    };

  } // namespace grpc
} // namespace gr

#endif /* INCLUDED_GRPC_MSG_SINK_IMPL_H */

