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
// Ported from https://github.com/artlav/meteor_decoder/blob/master/alib/ecc.pas

#ifndef INCLUDED_METEOR_ECC_H
#define INCLUDED_METEOR_ECC_H

#include <array>

namespace gr {
namespace starcoder {
namespace meteor {

void ecc_deinterleave(const uint8_t *data, uint8_t *output, int pos, int n);

void ecc_interleave(const uint8_t *data, uint8_t *output, int pos, int n);

int ecc_decode(uint8_t *data, int pad);

void ecc_encode(uint8_t *data, int pad);

}  // namespace meteor
}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_ECC_H */
