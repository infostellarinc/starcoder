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

#ifndef INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_H
#define INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_H

#include <starcoder_utils/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace starcoder_utils {

/*!
 * \brief <+description of block+>
 * \ingroup starcoder_utils
 *
 */
class STARCODER_UTILS_API folder_source_pdu : virtual public gr::sync_block {
 public:
  typedef boost::shared_ptr<folder_source_pdu> sptr;

  /*!
   * \brief Return a shared_ptr to a new instance of
   * starcoder_utils::folder_source_pdu.
   *
   * To avoid accidental use of raw pointers,
   * starcoder_utils::folder_source_pdu's
   * constructor is in a private implementation
   * class. starcoder_utils::folder_source_pdu::make is the public interface for
   * creating new instances.
   */
  static sptr make(const std::string& folder_name, int packet_length_bytes,
                   int delay_between_packets_ms);
};

}  // namespace starcoder_utils
}  // namespace gr

#endif /* INCLUDED_STARCODER_UTILS_FOLDER_SOURCE_PDU_H */
