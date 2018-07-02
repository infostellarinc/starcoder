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

#include "meteor_decoder.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "meteor_ecc.h"
#include "meteor_packet.h"

namespace gr {
namespace starcoder {

meteor_decoder::meteor_decoder()
    : correlator_(0xfca2b63db00d9794),  // sync word for meteor
      pos_(0),
      cpos_(0),
      word_(0),
      corr_(64),
      prev_pos_(0),
      sig_q_(0),
      last_sync_(0) {}

meteor_decoder::~meteor_decoder() {}

void meteor_decoder::do_next_correlate(unsigned char *raw,
                                       unsigned char *aligned) {
  cpos_ = 0;
  std::copy(raw + pos_, raw + pos_ + SOFT_FRAME_LEN, aligned);
  prev_pos_ = pos_;
  pos_ += SOFT_FRAME_LEN;

  correlator_.fix_packet(aligned, SOFT_FRAME_LEN, word_);
}

void meteor_decoder::do_full_correlate(unsigned char *raw,
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

bool meteor_decoder::try_frame(unsigned char *aligned, uint8_t *ecced_data) {
  uint8_t *decoded = new uint8_t[HARD_FRAME_LEN];
  uint8_t *ecc_buf = new uint8_t[255];

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
    ecc_interleave(ecc_buf, ecced_data, j, 4);
  }

  delete[] decoded;
  delete[] ecc_buf;

  return (ecc_results_[0] != -1) && (ecc_results_[1] != -1) &&
         (ecc_results_[2] != -1) && (ecc_results_[3] != -1);
}

bool meteor_decoder::decode_one_frame(unsigned char *raw, uint8_t *ecced_data) {
  unsigned char *aligned = new unsigned char[SOFT_FRAME_LEN];
  bool result = false;

  if (cpos_ == 0) {
    do_next_correlate(raw, aligned);

    result = try_frame(aligned, ecced_data);

    if (!result) pos_ -= SOFT_FRAME_LEN;
  }

  if (!result) {
    do_full_correlate(raw, aligned);

    result = try_frame(aligned, ecced_data);
  }

  delete[] aligned;
  return result;
}

}  // namespace starcoder
}  // namespace gr
