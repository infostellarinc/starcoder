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

#include "meteor_packet.h"

#include <iostream>
#include <algorithm>

namespace gr {
namespace starcoder {

meteor_packet::meteor_packet()
    : last_frame_(0), partial_packet_(false), packet_off_(0), packet_buf_(new uint8_t[2048]) {}

meteor_packet::~meteor_packet() {
  delete[] packet_buf_;
}

void meteor_packet::parse_70(uint8_t *packet, int len) {

}

void meteor_packet::act_apd(uint8_t *packet, int len, int apd, int pck_cnt) {

}

void meteor_packet::parse_apd(uint8_t *packet, int len) {
  uint16_t w = (packet[0] << 8) | packet[1];
  int sec = (w >> 11) & 1;
  int apd = w & 0x7ff;

  int pck_cnt = ((packet[2] << 8) | packet[3]) & 0x3fff;
  int len_pck = (packet[4] << 8) | packet[5];

  int ms = (packet[8] << 24) | (packet[9] << 16) | (packet[10] << 8) | packet[11];

  std::cout << "sec=" << sec << " (pck:" << len_pck+1 << "/total:" << len << " ms=" << ms << std::endl;

  if (apd == 70) parse_70(packet + 14, len -14);
  else act_apd(packet + 14, len - 14, apd, pck_cnt);
}

int meteor_packet::parse_partial(uint8_t *packet, int len) {
  if (len < 6) {
    partial_packet_ = true;
    return 0;
  }

  int len_pck = (packet[4] << 8) | packet[5];
  if (len_pck >= len-6) {
    partial_packet_ = true;
    return 0;
  }

  parse_apd(packet, len_pck + 1);

  partial_packet_ = false;
  return len_pck+6+1;
}

void meteor_packet::parse_cvcdu(uint8_t *frame, int len) {
  int n;

  uint16_t w = (frame[0] << 8) | frame[9];
  int ver = w >> 14;
  int ssid = (w >> 6) & 0xff;
  int fid = w & 0x3f;

  int frame_cnt = (frame[2] << 16) | (frame[3] << 8) | frame[4];

  w = (frame[8] << 8) | frame[9];
  uint8_t hdr_mark = w >> 11;
  uint16_t hdr_off = w & 0x7ff;

  std::cout << std::dec << "ver=" << ver << " ssid=" << ssid << " fid=" << fid << " frame_cnt=" <<
    frame_cnt << " hdr_mark=" << int(hdr_mark) << " hdr_off=" << hdr_off << std::endl;

  if (ver == 0 || fid == 0) return; // Empty packet

  int data_len = len - 10;
  if (frame_cnt == last_frame_+1) {
    if (partial_packet_) {
      if (hdr_off == PACKET_FULL_MARK) {
        hdr_off = len - 10;
        std::move(frame + 10, frame + 10 + hdr_off, packet_buf_ + packet_off_);
        packet_off_ += hdr_off;
      } else {
        std::move(frame + 10, frame + 10 + hdr_off, packet_buf_ + packet_off_);
        n = parse_partial(packet_buf_, packet_off_ + hdr_off);
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
      std::move(frame + 10 + off, frame + 10 + off + packet_off_, packet_buf_);
      break;
    } else {
      off += n;
      data_len -= n;
    }
  }
}

}  // namespace starcoder
}  // namespace gr
