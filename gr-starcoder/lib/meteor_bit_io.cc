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

#include "meteor_bit_io.h"

#include <iostream>

namespace gr {
namespace starcoder {

meteor_bit_io::meteor_bit_io(uint8_t *bytes, int len)
    : bytes_(bytes), len_(len), cur_(0), cur_len_(0), pos_(0) {}

meteor_bit_io::~meteor_bit_io() {}

uint32_t meteor_bit_io::bio_peek_n_bits(int n) {
  uint32_t result = 0;
  for (int i = 0; i < n; i++) {
    int p = pos_ + i;
    int bit = (bytes_[p >> 3] >> (7 - (p & 7))) & 1;
    result = (result << 1) | bit;
  }
  return result;
}

void meteor_bit_io::bio_advance_n_bits(int n) { pos_ += n; }

uint32_t meteor_bit_io::bio_fetch_n_bits(int n) {
  uint32_t result = bio_peek_n_bits(n);
  bio_advance_n_bits(n);
  return result;
}

void meteor_bit_io::bio_write_bitlist_reversed(uint8_t *list, int len) {
  list = list + len - 1;

  uint8_t *bytes = bytes_;
  int byte_index = pos_;

  uint16_t b;

  if (cur_len_ != 0) {
    int close_len = 8 - cur_len_;
    if (close_len >= len) close_len = len;

    b = cur_;

    for (int i = 0; i < close_len; i++) {
      b |= list[0];
      b = b << 1;
      list--;
    }

    len -= close_len;

    if ((cur_len_ + close_len) == 8) {
      b = b >> 1;
      bytes[byte_index] = b;
      byte_index++;
    } else {
      cur_ = b;
      cur_len_ += close_len;
    }
  }

  int full_bytes = len / 8;

  for (int i = 0; i < full_bytes; i++) {
    bytes[byte_index] =
        (*(list - 0) << 7) | (*(list - 1) << 6) | (*(list - 2) << 5) |
        (*(list - 3) << 4) | (*(list - 4) << 3) | (*(list - 5) << 2) |
        (*(list - 6) << 1) | (*(list - 7));
    byte_index++;
    list -= 8;
  }

  len -= 8 * full_bytes;

  b = 0;
  for (int i = 0; i < len; i++) {
    b |= list[0];
    b = b << 1;
    list--;
  }

  cur_ = b;
  pos_ = byte_index;
  cur_len_ = len;
}

}  // namespace starcoder
}  // namespace gr
