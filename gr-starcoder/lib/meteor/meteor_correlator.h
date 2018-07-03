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
// https://github.com/artlav/meteor_decoder/blob/master/correlator.pas

#ifndef INCLUDED_METEOR_CORRELATOR_H
#define INCLUDED_METEOR_CORRELATOR_H

#include <array>
#include <tuple>

namespace gr {
namespace starcoder {
namespace meteor {

const int PATTERN_SIZE = 64;
const int PATTERN_COUNT = 8;
const int CORR_LIMIT = 55;

class correlator {
 private:
  std::array<std::array<unsigned char, PATTERN_COUNT>, PATTERN_SIZE> patts_ {}
  ;
  std::array<int, PATTERN_COUNT> correlation_ {}
  , tmp_corr_ {}
  , position_ {}
  ;
  std::array<unsigned char, 256> rotate_iq_table_ {}
  , invert_iq_table_ {}
  ;
  std::array<std::array<int, 256>, 256> corr_table_ {}
  ;

  void init_corr_tables();
  unsigned char rotate_iq(unsigned char data, int shift);
  uint64_t flip_iq_qw(uint64_t data);
  uint64_t rotate_iq_qw(uint64_t data, int shift);
  void corr_set_patt(int n, uint64_t p);
  void corr_reset();

 public:
  correlator(uint64_t q_word);
  ~correlator();

  void fix_packet(unsigned char *data, int len, int shift);
  std::tuple<uint32_t, uint32_t, uint32_t> corr_correlate(
      const unsigned char *data, uint32_t d_word);
};

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_CORRELATOR_H */
