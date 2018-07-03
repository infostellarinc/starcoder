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
// Ported from
// https://github.com/artlav/meteor_decoder/blob/master/met_packet.pas

#include "meteor_packet.h"

#include <iostream>
#include <algorithm>

namespace gr {
namespace starcoder {
namespace meteor {

static const int PACKET_FULL_MARK = 2047;

packeter::packeter()
    : last_frame_(0),
      partial_packet_(false),
      packet_off_(0),
      first_time_(0),
      last_time_(0),
      no_time_yet_(true),
      imager_(68, 65, 64) {}

packeter::~packeter() {}

void packeter::parse_70(const uint8_t *packet, int len) {
  int h = packet[8];
  int m = packet[9];
  int s = packet[10];
  int ms = packet[11] * 4;

  last_time_ = h * 3600 * 1000 + m * 60 * 1000 + s * 1000 + ms;
  if (no_time_yet_) {
    no_time_yet_ = false;
    first_time_ = last_time_;
  }

  std::cout << "Onboard time: " << h << ":" << m << ":" << s << "." << ms
            << std::endl;
}

void packeter::act_apd(const uint8_t *packet, int len, int apd, int pck_cnt) {
  int mcu_id = packet[0];
  int scan_hdr = (packet[1] << 8) | packet[2];
  int seg_hdr = (packet[3] << 8) | packet[4];
  int q = packet[5];

  std::cout << std::dec << "apd=" << apd << " pck_cnt=" << pck_cnt
            << " mcu_id=" << mcu_id << " scan_hdr=" << scan_hdr
            << " seg_hdr=" << seg_hdr << " q=" << q << std::endl;

  imager_.dec_mcus(packet + 6, len - 6, apd, pck_cnt, mcu_id, q);
}

void packeter::parse_apd(const uint8_t *packet, int len) {
  uint16_t w = (packet[0] << 8) | packet[1];
  int sec = (w >> 11) & 1;
  int apd = w & 0x7ff;

  int pck_cnt = ((packet[2] << 8) | packet[3]) & 0x3fff;
  int len_pck = (packet[4] << 8) | packet[5];

  int ms =
      (packet[8] << 24) | (packet[9] << 16) | (packet[10] << 8) | packet[11];

  std::cout << "sec=" << sec << " (pck:" << len_pck + 1 << "/total:" << len
            << " ms=" << ms << std::endl;

  if (apd == 70)
    parse_70(packet + 14, len - 14);
  else
    act_apd(packet + 14, len - 14, apd, pck_cnt);
}

int packeter::parse_partial(const uint8_t *packet, int len) {
  if (len < 6) {
    partial_packet_ = true;
    return 0;
  }

  int len_pck = (packet[4] << 8) | packet[5];
  if (len_pck >= len - 6) {
    partial_packet_ = true;
    return 0;
  }

  parse_apd(packet, len_pck + 1);

  partial_packet_ = false;
  return len_pck + 6 + 1;
}

std::string packeter::dump_image() { return imager_.dump_image(); }

std::string packeter::dump_gray_image(int apid) {
  return imager_.dump_gray_image(apid);
}

void packeter::parse_cvcdu(const uint8_t *frame, int len) {
  int n;

  uint16_t w = (frame[0] << 8) | frame[1];
  int ver = w >> 14;
  int ssid = (w >> 6) & 0xff;
  int fid = w & 0x3f;

  int frame_cnt = (frame[2] << 16) | (frame[3] << 8) | frame[4];

  w = (frame[8] << 8) | frame[9];
  uint8_t hdr_mark = w >> 11;
  uint16_t hdr_off = w & 0x7ff;

  std::cout << std::dec << "ver=" << ver << " ssid=" << ssid << " fid=" << fid
            << " frame_cnt=" << frame_cnt << " hdr_mark=" << int(hdr_mark)
            << " hdr_off=" << hdr_off << std::endl;

  if (ver == 0 || fid == 0) return;  // Empty packet

  int data_len = len - 10;
  if (frame_cnt == last_frame_ + 1) {
    if (partial_packet_) {
      if (hdr_off == PACKET_FULL_MARK) {
        hdr_off = len - 10;
        std::move(frame + 10, frame + 10 + hdr_off,
                  packet_buf_.begin() + packet_off_);
        packet_off_ += hdr_off;
      } else {
        std::move(frame + 10, frame + 10 + hdr_off,
                  packet_buf_.begin() + packet_off_);
        n = parse_partial(packet_buf_.begin(), packet_off_ + hdr_off);
      }
    }
  } else {
    if (hdr_off == PACKET_FULL_MARK) return;
    partial_packet_ = false;
    packet_off_ = 0;
  }
  last_frame_ = frame_cnt;

  data_len -= hdr_off;
  int off = hdr_off;
  while (data_len > 0) {
    n = parse_partial(frame + 10 + off, data_len);
    if (partial_packet_) {
      packet_off_ = data_len;
      std::move(frame + 10 + off, frame + 10 + off + packet_off_,
                packet_buf_.begin());
      break;
    } else {
      off += n;
      data_len -= n;
    }
  }
}

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr
