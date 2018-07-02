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

#ifndef INCLUDED_METEOR_BIT_IO_H
#define INCLUDED_METEOR_BIT_IO_H

#include <array>

namespace gr {
namespace starcoder {

class meteor_bit_io {
 private:
  uint8_t *bytes_;
  int pos_, len_;

  uint8_t cur_;
  int cur_len_;

 public:
  meteor_bit_io(uint8_t *bytes, int len);
  ~meteor_bit_io();

  void bio_write_bitlist_reversed(uint8_t *list, int len);
  uint32_t bio_peek_n_bits(int n);
  void bio_advance_n_bits(int n);
  uint32_t bio_fetch_n_bits(int n);
};

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_METEOR_BIT_IO_H */
