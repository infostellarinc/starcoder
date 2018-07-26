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

#ifndef INCLUDED_METEOR_DECODER_H
#define INCLUDED_METEOR_DECODER_H

#include <array>
#include "meteor_correlator.h"
#include "meteor_viterbi.h"

namespace gr {
namespace starcoder {
namespace meteor {

const int HARD_FRAME_LEN = 1024;
const int FRAME_BITS = HARD_FRAME_LEN * 8;
const int SOFT_FRAME_LEN = FRAME_BITS * 2;

class decoder {
 private:
  bool do_full_correlate(const unsigned char *raw, int raw_len,
                         unsigned char *aligned);
  void do_next_correlate(const unsigned char *raw, unsigned char *aligned);
  bool try_frame(const unsigned char *aligned, uint8_t *error_corrected_data);

  correlator correlator_;
  viterbi viterbi_;

  uint32_t word_, cpos_, corr_, last_sync_;
  std::array<int, 4> ecc_results_;
  int sig_q_, pos_, prev_pos_;

 public:
  bool decode_one_frame(const unsigned char *raw, int raw_len,
                        uint8_t *error_corrected_data);

  decoder();
  ~decoder();

  uint32_t last_sync();
  int pos();
  int prev_pos();
};

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_DECODER_H */
