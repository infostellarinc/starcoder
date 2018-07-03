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
// https://github.com/artlav/decoder/blob/master/met_to_data.pas

#include "meteor_decoder.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <memory>

#include "meteor_ecc.h"
#include "meteor_packet.h"

namespace gr {
namespace starcoder {
namespace meteor {

static const int MIN_CORRELATION = 45;

static const std::array<uint8_t, 255> PRAND {
  0xff, 0x48, 0x0e, 0xc0, 0x9a, 0x0d, 0x70, 0xbc, 0x8e, 0x2c, 0x93, 0xad, 0xa7,
      0xb7, 0x46, 0xce, 0x5a, 0x97, 0x7d, 0xcc, 0x32, 0xa2, 0xbf, 0x3e, 0x0a,
      0x10, 0xf1, 0x88, 0x94, 0xcd, 0xea, 0xb1, 0xfe, 0x90, 0x1d, 0x81, 0x34,
      0x1a, 0xe1, 0x79, 0x1c, 0x59, 0x27, 0x5b, 0x4f, 0x6e, 0x8d, 0x9c, 0xb5,
      0x2e, 0xfb, 0x98, 0x65, 0x45, 0x7e, 0x7c, 0x14, 0x21, 0xe3, 0x11, 0x29,
      0x9b, 0xd5, 0x63, 0xfd, 0x20, 0x3b, 0x02, 0x68, 0x35, 0xc2, 0xf2, 0x38,
      0xb2, 0x4e, 0xb6, 0x9e, 0xdd, 0x1b, 0x39, 0x6a, 0x5d, 0xf7, 0x30, 0xca,
      0x8a, 0xfc, 0xf8, 0x28, 0x43, 0xc6, 0x22, 0x53, 0x37, 0xaa, 0xc7, 0xfa,
      0x40, 0x76, 0x04, 0xd0, 0x6b, 0x85, 0xe4, 0x71, 0x64, 0x9d, 0x6d, 0x3d,
      0xba, 0x36, 0x72, 0xd4, 0xbb, 0xee, 0x61, 0x95, 0x15, 0xf9, 0xf0, 0x50,
      0x87, 0x8c, 0x44, 0xa6, 0x6f, 0x55, 0x8f, 0xf4, 0x80, 0xec, 0x09, 0xa0,
      0xd7, 0x0b, 0xc8, 0xe2, 0xc9, 0x3a, 0xda, 0x7b, 0x74, 0x6c, 0xe5, 0xa9,
      0x77, 0xdc, 0xc3, 0x2a, 0x2b, 0xf3, 0xe0, 0xa1, 0x0f, 0x18, 0x89, 0x4c,
      0xde, 0xab, 0x1f, 0xe9, 0x01, 0xd8, 0x13, 0x41, 0xae, 0x17, 0x91, 0xc5,
      0x92, 0x75, 0xb4, 0xf6, 0xe8, 0xd9, 0xcb, 0x52, 0xef, 0xb9, 0x86, 0x54,
      0x57, 0xe7, 0xc1, 0x42, 0x1e, 0x31, 0x12, 0x99, 0xbd, 0x56, 0x3f, 0xd2,
      0x03, 0xb0, 0x26, 0x83, 0x5c, 0x2f, 0x23, 0x8b, 0x24, 0xeb, 0x69, 0xed,
      0xd1, 0xb3, 0x96, 0xa5, 0xdf, 0x73, 0x0c, 0xa8, 0xaf, 0xcf, 0x82, 0x84,
      0x3c, 0x62, 0x25, 0x33, 0x7a, 0xac, 0x7f, 0xa4, 0x07, 0x60, 0x4d, 0x06,
      0xb8, 0x5e, 0x47, 0x16, 0x49, 0xd6, 0xd3, 0xdb, 0xa3, 0x67, 0x2d, 0x4b,
      0xbe, 0xe6, 0x19, 0x51, 0x5f, 0x9f, 0x05, 0x08, 0x78, 0xc4, 0x4a, 0x66,
      0xf5, 0x58
}
;

decoder::decoder()
    : correlator_(0xfca2b63db00d9794),  // sync word for meteor
      pos_(0),
      cpos_(0),
      word_(0),
      corr_(64),
      prev_pos_(0),
      sig_q_(0),
      last_sync_(0) {}

decoder::~decoder() {}

void decoder::do_next_correlate(const unsigned char *raw,
                                unsigned char *aligned) {
  cpos_ = 0;
  std::copy(raw + pos_, raw + pos_ + SOFT_FRAME_LEN, aligned);
  prev_pos_ = pos_;
  pos_ += SOFT_FRAME_LEN;

  correlator_.fix_packet(aligned, SOFT_FRAME_LEN, word_);
}

void decoder::do_full_correlate(const unsigned char *raw,
                                unsigned char *aligned) {
  std::tie(word_, cpos_, corr_) =
      correlator_.corr_correlate(raw + pos_, SOFT_FRAME_LEN);

  if (corr_ < MIN_CORRELATION) {
    prev_pos_ = pos_;
    std::cout << "Not even " << MIN_CORRELATION << " bits found!";
    std::copy(raw + pos_, raw + pos_ + SOFT_FRAME_LEN, aligned);
    pos_ += SOFT_FRAME_LEN / 4;
  } else {
    prev_pos_ = pos_ + cpos_;
    std::copy(raw + pos_ + cpos_, raw + pos_ + cpos_ + SOFT_FRAME_LEN, aligned);
    pos_ += SOFT_FRAME_LEN + cpos_;
  }
  correlator_.fix_packet(aligned, SOFT_FRAME_LEN, word_);
}

bool decoder::try_frame(const unsigned char *aligned,
                        uint8_t *error_corrected_data) {
  std::unique_ptr<uint8_t[]> decoded_deleter(new uint8_t[HARD_FRAME_LEN]());
  uint8_t *decoded = decoded_deleter.get();
  std::unique_ptr<uint8_t[]> ecc_buf_deleter(new uint8_t[255]());
  uint8_t *ecc_buf = ecc_buf_deleter.get();

  viterbi_.vit_decode(aligned, decoded);

  last_sync_ = *reinterpret_cast<uint32_t *>(decoded);

  if (viterbi_.count_bits(last_sync_ ^ 0xE20330E5) <
      viterbi_.count_bits(last_sync_ ^ 0x1DFCCF1A)) {
    for (int j = 0; j < HARD_FRAME_LEN; j++) {
      decoded[j] = decoded[j] ^ 0xFFFFFFFF;
    }
    last_sync_ = last_sync_ ^ 0xFFFFFFFF;
  }

  for (int j = 0; j < HARD_FRAME_LEN - 4; j++) {
    decoded[4 + j] = decoded[4 + j] ^ PRAND[j % 255];
  }
  for (int j = 0; j < 4; j++) {
    ecc_deinterleave(decoded + 4, ecc_buf, j, 4);
    ecc_results_[j] = ecc_decode(ecc_buf, 0);
    ecc_interleave(ecc_buf, error_corrected_data, j, 4);
  }

  return (ecc_results_[0] != -1) && (ecc_results_[1] != -1) &&
         (ecc_results_[2] != -1) && (ecc_results_[3] != -1);
}

bool decoder::decode_one_frame(const unsigned char *raw,
                               uint8_t *error_corrected_data) {
  std::unique_ptr<uint8_t[]> u_aligned(new uint8_t[SOFT_FRAME_LEN]());
  uint8_t *aligned = u_aligned.get();
  bool result = false;

  if (cpos_ == 0) {
    do_next_correlate(raw, aligned);

    result = try_frame(aligned, error_corrected_data);

    if (!result) pos_ -= SOFT_FRAME_LEN;
  }

  if (!result) {
    do_full_correlate(raw, aligned);

    result = try_frame(aligned, error_corrected_data);
  }

  return result;
}

uint32_t decoder::last_sync() { return last_sync_; }

int decoder::pos() { return pos_; }

int decoder::prev_pos() { return prev_pos_; }

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr
