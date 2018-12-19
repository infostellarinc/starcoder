#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2018 Infostellar.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
import numpy as np


def pack_bits(arr):
    """
    Packs arr consisting of unpacked byte representation (uint8 1s and 0s)
    into a packed byte array
    """
    packed_arr = np.zeros(len(arr)/8, dtype=np.uint8)
    idx = 0
    packed_arr_idx = 0
    packed_byte = 0
    for bit in arr:
        packed_byte |= bit << (7-idx)
        if idx == 7:
            packed_arr[packed_arr_idx] = packed_byte
            packed_arr_idx += 1
            packed_byte = 0
            idx = 0
        else:
            idx += 1
    return packed_arr
