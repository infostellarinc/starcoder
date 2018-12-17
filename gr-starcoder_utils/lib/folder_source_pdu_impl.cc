/* -*- c++ -*- */
/* 
 * Copyright 2018 Infostellar.
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
#include "folder_source_pdu_impl.h"

namespace gr {
  namespace starcoder_utils {

    folder_source_pdu::sptr
    folder_source_pdu::make(const std::string& folder_name, int packet_length_bytes, int delay_between_packets_ms)
    {
      return gnuradio::get_initial_sptr
        (new folder_source_pdu_impl(folder_name, packet_length_bytes, delay_between_packets_ms));
    }

    /*
     * The private constructor
     */
    folder_source_pdu_impl::folder_source_pdu_impl(const std::string& folder_name, int packet_length_bytes, int delay_between_packets_ms)
      : gr::sync_block("folder_source_pdu",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        folder_name_(folder_name),
        packet_length_bytes_(packet_length_bytes),
        delay_between_packets_ms_(delay_between_packets_ms),
        finished_(false),
        out_port_(pmt::mp("out"))
    {
      message_port_register_out(out_port_);
    }

    /*
     * Our virtual destructor.
     */
    folder_source_pdu_impl::~folder_source_pdu_impl()
    {
    }

    bool folder_source_pdu_impl::start() {
      // NOTE: finished_ should be something explicitly thread safe. But since
      // nothing breaks on concurrent access, I'll just leave it as bool.
      finished_ = false;
      thread_ = boost::shared_ptr<gr::thread::thread>
        (new gr::thread::thread(boost::bind(&folder_source_pdu_impl::run, this)));

      return block::start();
    }

    bool folder_source_pdu_impl::stop() {
      finished_ = true;
      thread_->interrupt();
      thread_->join();

      return block::stop();
    }

    void folder_source_pdu_impl::run() {
      while(!finished_) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(delay_between_packets_ms_));
        if (finished_) return;

        message_port_pub(out_port_, pmt::mp("lolol"));
      }
    }

    int
    folder_source_pdu_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      return noutput_items;
    }

  } /* namespace starcoder_utils */
} /* namespace gr */

