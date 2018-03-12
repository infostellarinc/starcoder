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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <grpc++/grpc++.h>
#include "msg_sink_impl.h"
#include "grpcapi.grpc.pb.h"

using grpc::ClientContext;
using grpc::Status;
using grpcapi::MessageSink;
using grpcapi::SendMessageRequest;
using grpcapi::SendMessageResponse;

namespace gr {
  namespace grgrpc {

    msg_sink::sptr
    msg_sink::make(char* address)
    {
      return gnuradio::get_initial_sptr
        (new msg_sink_impl(address));
    }

    /*
     * The private constructor
     */
    msg_sink_impl::msg_sink_impl(char* address)
      : gr::block("msg_sink",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        // TODO: Add options for SSL
        stub_(MessageSink::NewStub(
            CreateChannel(address, grpc::InsecureChannelCredentials())))
    {
      message_port_register_in(pmt::mp("in"));
      set_msg_handler( pmt::mp("in"),
        boost::bind(&msg_sink_impl::handler, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    msg_sink_impl::~msg_sink_impl()
    {
      stub_.release();
    }

    void msg_sink_impl::handler(pmt::pmt_t msg)
    {
      std::stringbuf sb("");
      // TODO: Convert PMT to a gRPC native data structure.
      // Use built-in PMT serialization for now.
      pmt::serialize(msg, sb);

      SendMessageRequest req;
      SendMessageResponse resp;

      std::cout << "Sending message" << sb.str() << std::endl;

      req.set_bytes(sb.str());

      ClientContext context;
      // TODO: Add a timeout for sending
      Status status = stub_->SendMessage(&context, req, &resp);
      if (!status.ok()) {
        std::cout << "SendMessage rpc failed." << std::endl;
        return;
      }
      if (resp.status() != grpcapi::SendMessageResponse_Status_SUCCESS) {
        std::cout << "SendMessage rpc returned unsuccessful status" << std::endl;
      } else {
        std::cout << "SendMessage rpc returned successfully" << std::endl;
      }
    }

  } /* namespace grgrpc */
} /* namespace gr */
