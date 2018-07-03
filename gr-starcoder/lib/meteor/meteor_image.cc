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
// Ported from https://github.com/artlav/meteor_decoder/blob/master/met_jpg.pas

#include "meteor_image.h"
#include "meteor_bit_io.h"
#include "gil_util.h"

#include <iostream>
#include <cmath>

#include <boost/gil/gil_all.hpp>

namespace gr {
namespace starcoder {
namespace meteor {

imager::imager(int red_apid, int green_apid, int blue_apid)
    : red_apid_(red_apid),
      green_apid_(green_apid),
      blue_apid_(blue_apid),
      last_mcu_(-1),
      cur_y_(0),
      last_y_(-1),
      first_pck_(0),
      prev_pck_(0) {
  init_huffman_table();
  init_cos();
}

void imager::init_cos() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      cosine_[y][x] = cos(M_PI / 16 * (2 * y + 1) * x);
    }
  }
  for (int x = 0; x < 8; x++) {
    if (x == 0)
      alpha_[x] = 1 / sqrt(2);
    else
      alpha_[x] = 1;
  }

  //for (auto x : cosine_) for (auto y: x) std::cout << std::dec << y << ' ';
}

void imager::init_huffman_table() {
  std::array<uint8_t, 65536> v {}
  ;
  std::array<uint16_t, 17> min_code {}
  , maj_code {}
  ;

  int p = 16;
  for (int k = 1; k < 17; k++) {
    for (int i = 0; i < T_AC_0[k - 1]; i++) {
      v[(k << 8) + i] = T_AC_0[p];
      p++;
    }
  }

  uint32_t code = 0;
  for (int k = 1; k < 17; k++) {
    min_code[k] = code;
    code += T_AC_0[k - 1];
    maj_code[k] = code - (uint32_t)(code != 0);
    code *= 2;
    if (T_AC_0[k - 1] == 0) {
      min_code[k] = 0xffff;
      maj_code[k] = 0;
    }
  }

  int n = 0;
  for (int k = 1; k < 17; k++) {
    uint16_t min_val = min_code[k];
    uint16_t max_val = maj_code[k];
    for (int i = 0; i < (1 << k); i++) {
      if (i <= max_val && i >= min_val) {
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

  for (int i = 0; i < 65536; i++) {
    ac_lookup_[i] = get_ac_real(i);
  }
  for (int i = 0; i < 65536; i++) {
    dc_lookup_[i] = get_dc_real(i);
  }
}

int imager::get_dc_real(uint16_t word) {
  switch (word >> 14) {
    case 0:
      return 0;
    default:
      switch (word >> 13) {
        case 2:
          return 1;
        case 3:
          return 2;
        case 4:
          return 3;
        case 5:
          return 4;
        case 6:
          return 5;
        default:
          if ((word >> 12) == 0x00e) return 6;
          if ((word >> 11) == 0x01e) return 7;
          if ((word >> 10) == 0x03e) return 8;
          if ((word >> 9) == 0x07e) return 9;
          if ((word >> 8) == 0x0fe) return 10;
          if ((word >> 7) == 0x1fe) return 11;
      }
  }
  return -1;
}

int imager::get_ac_real(uint16_t word) {
  for (int i = 0; i < 162; i++) {
    if (((word >> (16 - ac_table_[i].len)) & ac_table_[i].mask) ==
        ac_table_[i].code) {
      return i;
    }
  }
  return -1;
}

imager::~imager() {}

std::string imager::dump_image() {
  if (full_image_.size() == 0) return "";

  const int width = 8 * MCU_PER_LINE;
  const int height = cur_y_ + 8;
  boost::gil::rgb8_image_t img(width, height);
  boost::gil::rgb8_image_t::view_t v = view(img);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int off = x + y * width;
      // To generate the full color image, we don't use APID-68 (red channel).
      v(x, y) = boost::gil::rgb8_pixel_t(full_image_[off].g, full_image_[off].g,
                                         full_image_[off].b);
    }
  }

  return store_rgb_to_png_string(v);
}

std::string imager::dump_gray_image(int apid) {
  if (full_image_.size() == 0) return "";

  const int width = 8 * MCU_PER_LINE;
  const int height = cur_y_ + 8;
  boost::gil::gray8_image_t img(width, height);
  boost::gil::gray8_image_t::view_t v = view(img);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int off = x + y * MCU_PER_LINE * 8;
      switch (apid) {
        case RED_APID:
          v(x, y) = boost::gil::gray8_pixel_t(full_image_[off].r);
          break;
        case GREEN_APID:
          v(x, y) = boost::gil::gray8_pixel_t(full_image_[off].g);
          break;
        case BLUE_APID:
          v(x, y) = boost::gil::gray8_pixel_t(full_image_[off].b);
          break;
      }
    }
  }

  return store_gray_to_png_string(v);
}

bool imager::progress_image(int apd, int mcu_id, int pck_cnt) {
  if (apd == 0 || apd == 70) return false;

  if (last_mcu_ == -1) {
    if (mcu_id != 0) return false;
    prev_pck_ = pck_cnt;
    first_pck_ = pck_cnt;
    if (apd == 65) first_pck_ -= 14;
    if (apd == 66) first_pck_ -= 28;
    if (apd == 68) first_pck_ -= 28;
    last_mcu_ = 0;
    cur_y_ = -1;
  }

  if (pck_cnt < prev_pck_) first_pck_ -= 16384;
  prev_pck_ = pck_cnt;

  cur_y_ = 8 * ((pck_cnt - first_pck_) / (14 + 14 + 14 + 1));
  if (cur_y_ > last_y_) full_image_.resize(MCU_PER_LINE * 8 * (cur_y_ + 8));
  last_y_ = cur_y_;

  return true;
}

void imager::fill_dqt_by_q(std::array<int, 64> &dqt, int q) {
  float f;
  if (q > 20 && q < 50)
    f = 5000. / q;
  else
    f = 200. - 2 * q;

  for (int i = 0; i < 64; i++) {
    dqt[i] = (int) round(f / 100. * STANDARD_QUANTIZATION_TABLE[i]);
    if (dqt[i] < 1) dqt[i] = 1;
  }
}

int imager::map_range(int cat, int vl) {
  int maxval = (1 << cat) - 1;
  bool sig = (vl >> (cat - 1)) != 0;
  if (sig)
    return vl;
  else
    return vl - maxval;
}

void imager::flt_idct_8x8(std::array<float, 64> &res,
                          std::array<float, 64> &inp) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      float s = 0;
      for (int u = 0; u < 8; u++) {
        float cxu = alpha_[u] * cosine_[x][u];
        s += cxu * (inp[0 * 8 + u] * alpha_[0] * cosine_[y][0] +
                    inp[1 * 8 + u] * alpha_[1] * cosine_[y][1] +
                    inp[2 * 8 + u] * alpha_[2] * cosine_[y][2] +
                    inp[3 * 8 + u] * alpha_[3] * cosine_[y][3] +
                    inp[4 * 8 + u] * alpha_[4] * cosine_[y][4] +
                    inp[5 * 8 + u] * alpha_[5] * cosine_[y][5] +
                    inp[6 * 8 + u] * alpha_[6] * cosine_[y][6] +
                    inp[7 * 8 + u] * alpha_[7] * cosine_[y][7]);
      }
      res[y * 8 + x] = s / 4;
    }
  }
}

void imager::fill_pix(std::array<float, 64> &img_dct, int apd, int mcu_id,
                      int m) {
  for (int i = 0; i < 64; i++) {
    int t = round(img_dct[i] + 128);
    if (t < 0) t = 0;
    if (t > 255) t = 255;
    int x = (mcu_id + m) * 8 + i % 8;
    int y = cur_y_ + i / 8;
    int off = x + y * MCU_PER_LINE * 8;

    if (apd == RED_APID) full_image_[off].r = t;
    if (apd == GREEN_APID) full_image_[off].g = t;
    if (apd == BLUE_APID) full_image_[off].b = t;
  }
}

void imager::dec_mcus(const uint8_t *packet, int len, int apd, int pck_cnt,
                      int mcu_id, uint8_t q) {
  bit_io_const b(packet, 0);

  if (!progress_image(apd, mcu_id, pck_cnt)) return;

  std::array<int, 64> dqt {}
  ;
  std::array<float, 64> zdct {}
  , dct {}
  , img_dct {}
  ;
  fill_dqt_by_q(dqt, q);

  float prev_dc = 0;
  int m = 0;
  while (m < MCU_PER_PACKET) {
    int dc_cat = dc_lookup_[b.peek_n_bits(16)];
    if (dc_cat == -1) {
      std::cerr << "Bad DC Huffman code!" << std::endl;
      return;
    }
    b.advance_n_bits(DC_CAT_OFF[dc_cat]);
    uint32_t n = b.fetch_n_bits(dc_cat);

    zdct[0] = map_range(dc_cat, n) + prev_dc;
    prev_dc = zdct[0];

    int k = 1;
    while (k < 64) {
      int ac = ac_lookup_[b.peek_n_bits(16)];
      if (ac == -1) {
        std::cerr << "Bad AC Huffman code!" << std::endl;
        return;
      }
      int ac_len = ac_table_[ac].len;
      int ac_size = ac_table_[ac].size;
      int ac_run = ac_table_[ac].run;
      b.advance_n_bits(ac_len);

      if (ac_run == 0 && ac_size == 0) {
        for (int i = k; i < 64; i++) {
          zdct[i] = 0;
        }
        break;
      }

      for (int i = 0; i < ac_run; i++) {
        zdct[k] = 0;
        k++;
      }

      if (ac_size != 0) {
        uint16_t n = b.fetch_n_bits(ac_size);
        zdct[k] = map_range(ac_size, n);
        k++;
      } else if (ac_run == 15) {
        zdct[k] = 0;
        k++;
      }
    }

    for (int i = 0; i < 64; i++) {
      dct[i] = zdct[ZIGZAG[i]] * dqt[i];
    }

    flt_idct_8x8(img_dct, dct);
    fill_pix(img_dct, apd, mcu_id, m);

    m++;
  }

}

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr
