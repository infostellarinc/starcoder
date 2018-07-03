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
// https://github.com/artlav/meteor_decoder/blob/master/correlator.pas

#include "meteor_correlator.h"

#include <iostream>
#include <algorithm>

namespace gr {
namespace starcoder {
namespace meteor {

correlator::correlator(uint64_t q_word) {
  init_corr_tables();
  for (int i = 0; i < 4; i++) {
    corr_set_patt(i, rotate_iq_qw(q_word, i));
  }
  for (int i = 0; i < 4; i++) {
    corr_set_patt(i + 4, rotate_iq_qw(flip_iq_qw(q_word), i));
  }
  //for (auto x : corr_table_[0]) std::cout << std::hex << (int)(x) << ' ';
  //for (auto x : rotate_iq_table_) std::cout << std::hex << (int)(x) << ' ';
  //for (auto x : invert_iq_table_) std::cout << std::hex << (int)(x) << ' ';
  //for (auto x : patts_) std::cout << std::hex << (int)(x[0]) << ' ';
}

correlator::~correlator() {}

void correlator::init_corr_tables() {
  for (int i = 0; i < 256; i++) {
    rotate_iq_table_[i] = (((i & 0x55) ^ 0x55) << 1) | ((i & 0xaa) >> 1);
    invert_iq_table_[i] = ((i & 0x55) << 1) | ((i & 0xaa) >> 1);
    for (int j = 0; j < 256; j++) {
      corr_table_[i][j] =
          (int)(((i > 127) && (j == 0)) || ((i <= 127) && (j == 255)));
    }
  }
}

unsigned char correlator::rotate_iq(unsigned char data, int shift) {
  if (shift == 1 || shift == 3) {
    data = rotate_iq_table_[data];
  }
  if (shift == 2 || shift == 3) {
    data = data ^ 0xff;
  }
  return data;
}

uint64_t correlator::rotate_iq_qw(uint64_t data, int shift) {
  uint64_t result = 0;
  unsigned char *presult = reinterpret_cast<unsigned char *>(&result);
  unsigned char *pdata = reinterpret_cast<unsigned char *>(&data);
  for (int i = 0; i < PATTERN_COUNT; i++) {
    presult[i] = rotate_iq(pdata[i], shift);
  }
  return result;
}

uint64_t correlator::flip_iq_qw(uint64_t data) {
  uint64_t result = 0;
  unsigned char *presult = reinterpret_cast<unsigned char *>(&result);
  unsigned char *pdata = reinterpret_cast<unsigned char *>(&data);
  for (int i = 0; i < PATTERN_COUNT; i++) {
    presult[i] = invert_iq_table_[pdata[i]];
  }
  return result;
}

void correlator::corr_set_patt(int n, uint64_t p) {
  for (int i = 0; i < PATTERN_SIZE; i++) {
    if (((p >> (PATTERN_SIZE - i - 1)) & 1) != 0)
      patts_[i][n] = 0xff;
    else
      patts_[i][n] = 0;
  }
}

void correlator::fix_packet(unsigned char *data, int len, int shift) {
  signed char *d = reinterpret_cast<signed char *>(data);
  signed char b;

  switch (shift) {
    case 4:
      for (int j = 0; j < (len / 2); j++) {
        b = d[j * 2];
        d[j * 2] = d[j * 2 + 1];
        d[j * 2 + 1] = b;
      }
      break;
    case 5:
      for (int j = 0; j < (len / 2); j++) {
        d[j * 2] = -d[j * 2];
      }
      break;
    case 6:
      for (int j = 0; j < (len / 2); j++) {
        b = d[j * 2];
        d[j * 2] = -d[j * 2 + 1];
        d[j * 2 + 1] = -b;
      }
      break;
    case 7:
      for (int j = 0; j < (len / 2); j++) {
        d[j * 2 + 1] = -d[j * 2 + 1];
      }
      break;
  }
}

void correlator::corr_reset() {
  correlation_.fill(0);
  position_.fill(0);
  tmp_corr_.fill(0);
}

std::tuple<uint32_t, uint32_t, uint32_t> correlator::corr_correlate(
    const unsigned char *data, uint32_t len) {
  corr_reset();

  for (int i = 0; i < len - PATTERN_SIZE; i++) {

    tmp_corr_.fill(0);

    for (int k = 0; k < PATTERN_SIZE; k++) {
      for (int l = 0; l < PATTERN_COUNT; l++)
        tmp_corr_[l] += corr_table_[data[i + k]][patts_[k][l]];
    }

    for (int n = 0; n < PATTERN_COUNT; n++) {
      if (tmp_corr_[n] > correlation_[n]) {
        correlation_[n] = tmp_corr_[n];
        position_[n] = i;
        tmp_corr_[n] = 0;
        if (correlation_[n] > CORR_LIMIT) {
          return std::make_tuple(n, position_[n], correlation_[n]);
        }
      }
    }
  }

  int result =
      std::distance(correlation_.begin(),
                    std::max_element(correlation_.begin(), correlation_.end()));
  return std::make_tuple(result, position_[result], correlation_[result]);
}

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr
