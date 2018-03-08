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

#ifndef INCLUDED_GRGRPC_MSG_SINK_IMPL_H
#define INCLUDED_GRGRPC_MSG_SINK_IMPL_H

#include <grpc/grpc.h>
#include <grgrpc/msg_sink.h>
#include "grpcapi.grpc.pb.h"

using grpcapi::MessageSink;

namespace gr {
  namespace grgrpc {

    class msg_sink_impl : public msg_sink
    {
     private:
       std::unique_ptr<MessageSink::Stub> stub_;

     public:
      msg_sink_impl(const std::string& address);
      ~msg_sink_impl();

      void handler(pmt::pmt_t msg);
    };

  } // namespace grgrpc
} // namespace gr

#endif /* INCLUDED_GRGRPC_MSG_SINK_IMPL_H */

