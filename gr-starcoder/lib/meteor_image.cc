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

#include "meteor_image.h"

#include <iostream>

namespace gr {
namespace starcoder {

meteor_image::meteor_image(int red_apid, int green_apid, int blue_apid)
    : red_apid_(red_apid), green_apid_(green_apid), blue_apid_(blue_apid),
      last_mcu_(-1), cur_y_(0), last_y_(-1), first_pck_(0), prev_pck_(0)  {
  init_huffman_table();
}

void meteor_image::init_huffman_table() {
  std::array<uint8_t, 65536> v{};
  std::array<uint16_t, 17> min_code{}, maj_code{};

  int p = 16;
  for (int k=1; k<17; k++) {
    for (int i=0; i<T_AC_0[k-1]; i++) {
      v[(k << 8)+i] = T_AC_0[p];
      p++;
    }
  }

  uint32_t code = 0;
  for (int k=1; k<17; k++) {
    min_code[k] = code;
    code += T_AC_0[k-1];
    maj_code[k] = code - (uint32_t)(code!=0);
    code *= 2;
    if (T_AC_0[k-1] == 0) {
      min_code[k] = 0xffff;
      maj_code[k] = 0;
    }
  }

  int n =0;
  for (int k=1; k<17; k++) {
    uint16_t min_val = min_code[k];
    uint16_t max_val = maj_code[k];
    for (int i=0; i< (1 << k); i++) {
      if (i <=max_val && i>= min_val) {
        uint16_t size_val = v[(k << 8) + i - min_val];
        int run = size_val >> 4;
        int size = size_val & 0xf;
        ac_table_[n].run = run;
        ac_table_[n].size = size;
        ac_table_[n].len = k;
        ac_table_[n].mask = (1 << k) - 1;
        ac_table_[n].code = i;
        n++;
      }
    }
  }

  for (int i=0; i<65536; i++) {
    ac_lookup_[i] = get_ac_real(i);
  }
  for (int i=0; i<65536; i++) {
    dc_lookup_[i] = get_dc_real(i);
  }

  //for (int i=65500; i< 65536; i++) std::cout << std::hex << ac_lookup_[i] << ' ';
  //for (int i=0; i< 65536; i++) std::cout << std::hex << dc_lookup_[i] << ' ';
  //for (auto x : ac_table_) std::cout << std::hex << x.code << ' ';
}

int meteor_image::get_dc_real(uint16_t word) {
  switch (word >> 14) {
  case 0:
    return 0;
  default:
    switch (word >> 13) {
    case 2: return 1;
    case 3: return 2;
    case 4: return 3;
    case 5: return 4;
    case 6: return 5;
    default:
      if ((word>>12) == 0x00e) return 6;
      if ((word>>11) == 0x01e) return 7;
      if ((word>>10) == 0x03e) return 8;
      if ((word>>9) == 0x07e) return 9;
      if ((word>>8) == 0x0fe) return 10;
      if ((word>>7) == 0x1fe) return 11;
    }
  }
  return -1;
}

int meteor_image::get_ac_real(uint16_t word) {
  for (int i=0; i<162; i++) {
    if (((word >> (16-ac_table_[i].len)) & ac_table_[i].mask) == ac_table_[i].code) {
      return i;
    }
  }
  return -1;
}

meteor_image::~meteor_image() {}

}  // namespace starcoder
}  // namespace gr


int main() {
  gr::starcoder::meteor_image m(68, 65, 64);
}
