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
/*
 * Copyright 2017 Artyom Litvinovich
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

#ifndef INCLUDED_METEOR_PACKET_H
#define INCLUDED_METEOR_PACKET_H

#include <cstdint>

#include "meteor_image.h"

namespace gr {
namespace starcoder {

const int PACKET_FULL_MARK = 2047;

class meteor_packet {
 private:
  int last_frame_;
  bool partial_packet_;
  uint8_t *packet_buf_;
  int packet_off_;
  int first_time_, last_time_;
  bool no_time_yet_;
  meteor_image meteor_image_;

  int parse_partial(uint8_t *packet, int len);
  void parse_apd(uint8_t *packet, int len);
  void act_apd(uint8_t *packet, int len, int apd, int pck_cnt);
  void parse_70(uint8_t *packet, int len);

 public:
  meteor_packet();
  ~meteor_packet();

  void parse_cvcdu(uint8_t *frame, int len);
  std::string dump_image();
  std::string dump_gray_image(int apid);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_PACKET_H */
