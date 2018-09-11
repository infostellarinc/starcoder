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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "command_source_impl.h"

#include "proto_to_pmt.h"

namespace gr {
namespace starcoder {

command_source::sptr command_source::make() {
  return gnuradio::get_initial_sptr(new command_source_impl());
}

/*
 * The private constructor
 */
command_source_impl::command_source_impl()
    : gr::block("command_source", gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      finished_(false),
      port_(pmt::mp("out")),
      queue_(10485760) {
  message_port_register_out(port_);
}

bool command_source_impl::start() {
  finished_ = false;
  thread_ = new std::thread(std::bind(&command_source_impl::readloop, this));
  return true;
}

bool command_source_impl::stop() {
  finished_ = true;
  queue_.close();
  thread_->join();
  return true;
}

void command_source_impl::readloop() {
  while (!finished_) {
    for (std::string s = queue_.blocking_pop(); s.size() != 0;
         s = queue_.pop()) {
      ::starcoder::BlockMessage grpc_pmt;
      if (!grpc_pmt.ParseFromString(s)) {
        GR_LOG_WARN(d_logger, "Failed to deserialize gRPC");
        continue;
      }
      pmt::pmt_t m = convert_proto_to_pmt(grpc_pmt);
      message_port_pub(port_, m);
    }
  }
}

void command_source_impl::push(const std::string &message) {
  queue_.push(message);
}

uint64_t command_source_impl::get_starcoder_queue_ptr() {
  return reinterpret_cast<uint64_t>(&queue_);
}

/*
 * Our virtual destructor.
 */
command_source_impl::~command_source_impl() {}

} /* namespace starcoder */
} /* namespace gr */
