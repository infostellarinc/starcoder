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
#include "enqueue_message_sink_impl.h"

namespace gr {
namespace starcoder {

enqueue_message_sink::sptr enqueue_message_sink::make() {
  return gnuradio::get_initial_sptr(new enqueue_message_sink_impl());
}

/*
 * The private constructor
 */
enqueue_message_sink_impl::enqueue_message_sink_impl()
    : gr::sync_block("enqueue_message_sink", gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(0, 0, 0)),
      string_queue_(NULL) {
  message_port_register_in(pmt::mp("in"));
  set_msg_handler(pmt::mp("in"),
                  boost::bind(&enqueue_message_sink_impl::handler, this, _1));
}

/*
 * Our virtual destructor.
 */
enqueue_message_sink_impl::~enqueue_message_sink_impl() {}

void enqueue_message_sink_impl::handler(pmt::pmt_t msg) {
  if (string_queue_ != NULL) {
    std::string serialized = pmt::serialize_str(msg);
    if (serialized.length() > 10485760) {
      GR_LOG_ERROR(d_logger,
                   boost::format("Received large packet of length %d in "
                                 "enqueue_message_sink_impl::handler") %
                       serialized.length());
      return;
    }
    string_queue_->push(serialized);
  }
}

int enqueue_message_sink_impl::work(int noutput_items,
                                    gr_vector_const_void_star &input_items,
                                    gr_vector_void_star &output_items) {
  return noutput_items;
}

void enqueue_message_sink_impl::register_starcoder_queue(uint64_t ptr) {
  string_queue_ = reinterpret_cast<string_queue *>(ptr);
}

} /* namespace starcoder */
} /* namespace gr */
