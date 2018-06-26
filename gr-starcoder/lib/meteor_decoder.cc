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

#include "meteor_decoder.h"

#include <iostream>
#include <algorithm>

namespace gr {
namespace starcoder {

meteor_decoder::meteor_decoder() :
    correlator_(0xfca2b63db00d9794), // sync word for meteor
    pos_(0),
    cpos_(0),
    word_(0),
    corr_(64),
    prev_pos_(0),
    sig_q_(0),
    last_sync_(0)
{}

meteor_decoder::~meteor_decoder() {}

void meteor_decoder::do_next_correlate(unsigned char *raw, unsigned char *aligned) {
  cpos_ = 0;
  std::copy(raw + pos_, raw + pos_ + SOFT_FRAME_LEN, aligned);
  prev_pos_ = pos_;
  pos_ += SOFT_FRAME_LEN;

  correlator_.fix_packet(aligned, SOFT_FRAME_LEN, word_);
}

void meteor_decoder::do_full_correlate(unsigned char *raw, unsigned char *aligned) {
  std::tie(word_, cpos_, corr_) = correlator_.corr_correlate(raw + pos_, SOFT_FRAME_LEN);
}

bool meteor_decoder::try_frame(unsigned char *aligned) {
  // TODO: Do something
}

bool meteor_decoder::decode_one_frame(unsigned char *raw) {
  unsigned char *aligned = new unsigned char[SOFT_FRAME_LEN];
  bool result = false;

  if (cpos_ == 0) {
    do_next_correlate(raw, aligned);

    result = try_frame(aligned);

    if (!result) pos_ -= SOFT_FRAME_LEN;
  }

  if (!result) {
    do_full_correlate(raw, aligned);

    result = try_frame(aligned);
  }

  delete[] aligned;
  return result;
}

} // namespace starcoder
} // namespace gr

int main() {
  gr::starcoder::meteor_decoder a;
  gr::starcoder::meteor_correlator c(0xfca2b63db00d9794);
  unsigned char *p = new unsigned char[10];
  for (int i=0; i<10; i++) p[i] = i;
  c.fix_packet(p, 10, 7);
  for (int i=0; i<10; i++) std::cout << std::hex << (int)(p[i]) << ' ';
  delete[] p;
}
