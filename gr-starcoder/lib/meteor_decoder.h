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

#ifndef INCLUDED_METEOR_DECODER_H
#define INCLUDED_METEOR_DECODER_H

#include <array>
#include "meteor_correlator.h"

namespace gr {
namespace starcoder {

const int HARD_FRAME_LEN=1024;
const int FRAME_BITS=HARD_FRAME_LEN*8;
const int SOFT_FRAME_LEN=FRAME_BITS*2;
const int MIN_CORRELATION=45;

class meteor_decoder {
  private:
    meteor_correlator correlator_;
    int pos_, prev_pos_;
    std::array<unsigned char, HARD_FRAME_LEN> ecced_data_{};
    uint32_t word_, cpos_, corr_, last_sync_;
    std::array<int, 4> r_;
    int sig_q_;

  public:
    bool decode_one_frame(unsigned char *raw);
    void do_full_correlate(unsigned char *raw, unsigned char *aligned);
    void do_next_correlate(unsigned char *raw, unsigned char *aligned);
    bool try_frame(unsigned char *aligned);

    meteor_decoder();
    ~meteor_decoder();
};

} // namespace starcoder
} // namespace gr

#endif /* INCLUDED_METEOR_DECODER_H */
