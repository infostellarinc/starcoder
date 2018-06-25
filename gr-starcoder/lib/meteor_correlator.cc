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

#include "meteor_correlator.h"

#include <iostream>

namespace gr {
namespace starcoder {

meteor_correlator::meteor_correlator(uint64_t q_word) {
  init_corr_tables();
  for (auto x : rotate_iq_table_) std::cout << std::hex << (int)(x) << ' ';
}

meteor_correlator::~meteor_correlator() {}

void meteor_correlator::init_corr_tables() {
  for (int i = 0; i<256; i++) {
    rotate_iq_table_[i] = (((i & 0x55) ^ 0x55) << 1) | ((i & 0xaa) >> 1);
    invert_iq_table_[i] = ((i & 0x55) << 1) | ((i & 0xaa) >> 1);
    for (int j=0;j<256;j++) {
      corr_table_[i][j] = (int) (((i > 127) && (j == 0)) || ((i <= 127) && (j == 255)));
    }
  }
}

uint64_t meteor_correlator::rotate_iq_qw(uint64_t data, int shift) {

}

} // namespace starcoder
} // namespace gr

int main() {
  gr::starcoder::meteor_correlator a(0xfca2b63db00d9794);
}
