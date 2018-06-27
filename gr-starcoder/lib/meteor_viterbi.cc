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

#include "meteor_viterbi.h"

#include <iostream>
#include <stdlib.h>

namespace gr {
namespace starcoder {

meteor_viterbi::meteor_viterbi() :
    ber_(0),
    err_index_(0),
    hist_index_(0),
    len_(0),
    pair_outputs_len_(5),
    renormalize_counter_(0)
{
  for (int i=0;i<4;i++) {
    for (int j=0;j<65536;j++) {
      dist_table_[i][j] = metric_soft_distance(i, j & 0xff, j >> 8);
    }
  }

  for (int i=0; i< 128; i++) {
    if ((count_bits(i & VITERBI27_POLYA) % 2) != 0) table_[i] = table_[i] | 1;
    if ((count_bits(i & VITERBI27_POLYB) % 2) != 0) table_[i] = table_[i] | 2;
  }

  pair_lookup_create();

  // for (auto x : dist_table_) std::cout << std::hex << (int)(x[60000]) << ' ';
  // for (auto x: table_) std::cout << std::hex << (int)(x) << ' ';
  // for (auto x: pair_keys_) std::cout << std::hex << (int)(x) << ' ';
  // for (auto x: pair_outputs_) std::cout << std::hex << (int)(x) << ' ';
}

uint16_t meteor_viterbi::metric_soft_distance(unsigned char hard, unsigned char soft_y0, unsigned char soft_y1) {
  const int mag = 255;
  int soft_x0, soft_x1;
  switch (hard & 3) {
  case 0:
    soft_x0 = mag;
    soft_x1 = mag;
    break;
  case 1:
    soft_x0 = -mag;
    soft_x1 = mag;
    break;
  case 2:
    soft_x0 = mag;
    soft_x1 = -mag;
    break;
  case 3:
    soft_x0 = -mag;
    soft_x1 = -mag;
    break;
  default:
    // Warn?
    soft_x0 = 0;
    soft_x1 = 0;
  }

  signed char y0 = reinterpret_cast<signed char&>(soft_y0);
  signed char y1 = reinterpret_cast<signed char&>(soft_y1);

  return abs(y0 - soft_x0) + abs(y1 - soft_x1);
}

void meteor_viterbi::pair_lookup_create() {
  std::array<uint32_t, 16> inv_outputs{};
  uint32_t output_counter = 1;
  uint32_t o;

  for (int i=0;i<64;i++) {
    o = (table_[i*2 + 1] << 2) | table_[i*2];
    if (inv_outputs[o] == 0) {
      inv_outputs[o] = output_counter;
      pair_outputs_[output_counter] = o;
      output_counter++;
    }
    pair_keys_[i] = inv_outputs[o];
  }
}

int meteor_viterbi::count_bits(uint32_t i) {
  // https://stackoverflow.com/a/109025/5636655
  i = i - ((i >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

meteor_viterbi::~meteor_viterbi() {}

} // namespace starcoder
} // namespace gr

/*
int main() {
  gr::starcoder::meteor_viterbi v;
}
*/
