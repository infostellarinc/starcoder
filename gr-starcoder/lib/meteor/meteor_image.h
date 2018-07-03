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

#ifndef INCLUDED_METEOR_IMAGE_H
#define INCLUDED_METEOR_IMAGE_H

#include <array>
#include <vector>

namespace gr {
namespace starcoder {
namespace meteor {

const int MCU_PER_PACKET = 14;
const int MCU_PER_LINE = 196;

const std::array<uint8_t, 64> STANDARD_QUANTIZATION_TABLE {
  16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55, 14, 13, 16,
      24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62, 18, 22, 37, 56, 68,
      109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92, 49, 64, 78, 87, 103, 121,
      120, 101, 72, 92, 95, 98, 112, 100, 103, 99
}
;

const std::array<uint8_t, 64> ZIGZAG {
  0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30,
      41, 43, 9, 11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20,
      22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48,
      49, 57, 58, 62, 63
}
;

const std::array<uint8_t, 178> T_AC_0 {
  0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125, 1, 2, 3, 0, 4, 17, 5, 18,
      33, 49, 65, 6, 19, 81, 97, 7, 34, 113, 20, 50, 129, 145, 161, 8, 35, 66,
      177, 193, 21, 82, 209, 240, 36, 51, 98, 114, 130, 9, 10, 22, 23, 24, 25,
      26, 37, 38, 39, 40, 41, 42, 52, 53, 54, 55, 56, 57, 58, 67, 68, 69, 70,
      71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99, 100, 101, 102, 103,
      104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 131, 132, 133, 134,
      135, 136, 137, 138, 146, 147, 148, 149, 150, 151, 152, 153, 154, 162, 163,
      164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182, 183, 184, 185,
      186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211, 212, 213, 214,
      215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 241,
      242, 243, 244, 245, 246, 247, 248, 249, 250
}
;

const std::array<int, 12> DC_CAT_OFF { 2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9 }
;

// APID is the channel identifier for Meteor M2
const int RED_APID = 68;
const int GREEN_APID = 65;
const int BLUE_APID = 64;

struct pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct ac_table_rec {
  int run;
  int size;
  int len;
  uint32_t mask;
  uint32_t code;
};

class imager {
 private:
  int red_apid_, green_apid_, blue_apid_;
  std::vector<pixel> full_image_;
  int last_mcu_, cur_y_, last_y_, first_pck_, prev_pck_;
  std::array<int, 65536> ac_lookup_ {}
  , dc_lookup_ {}
  ;
  std::array<ac_table_rec, 162> ac_table_ {}
  ;
  std::array<std::array<float, 8>, 8> cosine_ {}
  ;
  std::array<float, 8> alpha_;

  void init_huffman_table();
  void init_cos();
  int get_dc_real(uint16_t word);
  int get_ac_real(uint16_t word);
  bool progress_image(int apd, int mcu_id, int pck_cnt);
  void fill_dqt_by_q(std::array<int, 64> &dqt, int q);
  int map_range(int cat, int vl);
  void flt_idct_8x8(std::array<float, 64> &res, std::array<float, 64> &inp);
  void fill_pix(std::array<float, 64> &img_dct, int apd, int mcu_id, int m);

 public:
  imager(int red_apid, int green_apid, int blue_apid);
  ~imager();

  void dec_mcus(const uint8_t *packet, int len, int apd, int pck_cnt,
                int mcu_id, uint8_t q);

  std::string dump_image();
  std::string dump_gray_image(int apid);

};

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_IMAGE_H */
