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

#ifndef INCLUDED_METEOR_CORRELATOR_H
#define INCLUDED_METEOR_CORRELATOR_H

#include <array>

namespace gr {
namespace starcoder {

const int PATTERN_SIZE = 64;
const int PATTERN_COUNT = 8;
const int CORR_LIMIT = 55;

class meteor_correlator {
  private:
    std::array<std::array<unsigned char, PATTERN_COUNT>, PATTERN_SIZE> patts_{};
    std::array<int, PATTERN_COUNT> correlation_{}, tmp_corr_{}, position_{};
    std::array<unsigned char, 256> rotate_iq_table_{}, invert_iq_table_{};
    std::array<std::array<int, 256>, 256> corr_table_{};

    void init_corr_tables();
    uint64_t rotate_iq_qw(uint64_t data, int shift);

  public:
    meteor_correlator(uint64_t q_word);
    ~meteor_correlator();
};

} // namespace starcoder
} // namespace gr

#endif /* INCLUDED_METEOR_CORRELATOR_H */