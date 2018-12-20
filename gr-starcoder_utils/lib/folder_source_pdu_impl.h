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

#ifndef INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_IMPL_H
#define INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_IMPL_H

#include <starcoder_utils/folder_source_pdu.h>

namespace gr {
namespace starcoder_utils {

class folder_source_pdu_impl : public folder_source_pdu {
 private:

  const std::string folder_name_;
  int packet_length_bytes_;
  int delay_between_packets_ms_;
  bool finished_;
  boost::shared_ptr<gr::thread::thread> thread_;
  const pmt::pmt_t out_port_;

 public:
  folder_source_pdu_impl(const std::string &folder_name,
                         int packet_length_bytes, int delay_between_packets_ms);
  ~folder_source_pdu_impl();

  bool start();

  void run();
  bool stop();

  // Where all the action really happens
  int work(int noutput_items, gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

}  // namespace starcoder_utils
}  // namespace gr

#endif /* INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_IMPL_H */
