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

meteor_viterbi::~meteor_viterbi() {}

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

void meteor_viterbi::vit_decode(unsigned char *in, unsigned char *out) {

}

void meteor_viterbi::vit_conv_decode(unsigned char *soft_encoded, unsigned char *decoded) {
  // TODO: bit writer create
  len_ = 0;
  hist_index_ = 0;
  renormalize_counter_ = 0;

  errors_[0].fill(0);
  errors_[1].fill(0);
  err_index_ = 0;
  read_errors_ = errors_[0];
  write_errors_ = errors_[1];

  // TODO: vit inner
  // TODO: vit tail

  // TODO: history buffer traceback
}

void meteor_viterbi::vit_inner(unsigned char *soft) {
  for (int i=0; i<6; i++) {
    for (int j=0; j<(1 << (i+1)); j++) {
      write_errors_[j] =
          dist_table_[table_[j]][*reinterpret_cast<uint16_t *>(soft + i*2)] + read_errors_[j >> 1];
    }
    error_buffer_swap();
  }

  for (int i=6; i < FRAME_BITS-6; i++) {
    for(int j=0; j<4; j++) {
      distances_[j] = dist_table_[j][*reinterpret_cast<uint16_t *>(soft + i*2)];
    }
    pair_lookup_fill_distance();

    uint32_t highbase = HIGH_BIT >> 1;
    uint32_t low = 0;
    uint32_t high = HIGH_BIT;
    uint32_t base = 0;
    while (high < NUM_ITER) {
      uint32_t offset = 0;
      uint32_t base_offset = 0;
      while (base_offset < 4) {
        uint32_t low_key = pair_keys_[base + base_offset];
        uint32_t high_key = pair_keys_[highbase + base + base_offset];

        uint32_t low_concat_dist = pair_distances_[low_key];
        uint32_t high_concat_dist = pair_distances_[high_key];

        uint16_t low_past_error = read_errors_[base + base_offset];
        uint16_t high_past_error = read_errors_[highbase + base + base_offset];

        uint16_t low_error = (low_concat_dist & 0xffff) + low_past_error;
        uint16_t high_error = (high_concat_dist & 0xffff) + high_past_error;

        uint32_t successor = low + offset;

        uint16_t error;
        uint8_t history_mask;
        if (low_error <= high_error) {
          error = low_error;
          history_mask = 0;
        } else {
          error = high_error;
          history_mask = 1;
        }
        write_errors_[successor] = error;
        history_[hist_index_][successor] = history_mask;

        uint32_t low_plus_one = low + offset + 1;

        uint16_t low_plus_one_error = (low_concat_dist >> 16) + low_past_error;
        uint16_t high_plus_one_error = (high_concat_dist >> 16) + high_past_error;

        uint32_t plus_one_successor = low_plus_one;
        uint16_t plus_one_error;
        uint8_t plus_one_history_mask;
        if (low_plus_one_error <= high_plus_one_error) {
          plus_one_error = low_plus_one_error;
          plus_one_history_mask = 0;
        } else {
          plus_one_error = high_plus_one_error;
          plus_one_history_mask = 1;
        }
        write_errors_[plus_one_successor] = plus_one_error;
        history_[hist_index_][plus_one_successor] = plus_one_history_mask;

        offset += 2;
        base_offset += 1;
      }

      low += 8;
      high += 8;
      base += 4;
    }

    // todo history buffer process skip
    error_buffer_swap();
  }
}

void meteor_viterbi::pair_lookup_fill_distance() {
  for (int i=1; i<pair_outputs_len_; i++) {
    uint32_t c = pair_outputs_[i];
    uint32_t i0 = c & 3;
    uint32_t i1 = c >> 2;

    pair_distances_[i] = (distances_[i1] << 16) | distances_[i0];
  }
}

void meteor_viterbi::error_buffer_swap() {
  read_errors_ = errors_[err_index_];
  err_index_ = (err_index_+1) % 2;
  write_errors_ = errors_[err_index_];
}

} // namespace starcoder
} // namespace gr


int main() {
  gr::starcoder::meteor_viterbi v;
}

