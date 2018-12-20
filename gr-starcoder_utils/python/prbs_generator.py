#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright 2015 Free Software Foundation, Inc
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
import operator, copy


class PRBSGenerator:
    modes = {
        "PRBS7":[0,6,7],
        "PRBS15":[0,14,15],
        "PRBS23":[0,18,23],
        "PRBS31":[0,28,31]
    }

    def __init__(self, which_mode = "PRBS31", reset_len=100000):
        # https://en.wikipedia.org/wiki/Pseudorandom_binary_sequence
        self.gen_poly = self.modes[which_mode]
        self.reg  = np.ones([max(self.gen_poly)+1], dtype='uint8')
        self.reg[0::4] = 0  # pick a seed that likely doesnt start with long strings of 0s or 1s for sanity sake
        self.init_reg = copy.copy(self.reg)
        self.idx = 0
        self.reset_len = reset_len
        self.pregen()

    def pregen(self):
        self.pre = self.generate_n_bits(self.reset_len, lookup=False)

    def generate_n_bits_and_store(self, n=1000):
        o = np.zeros([n],dtype='uint8')
        for i in range(0, n):
            nv = reduce(operator.xor, map(lambda x: self.reg[x], self.gen_poly[1:]))
            self.reg[1:] = self.reg[0:-1]
            self.reg[0] = nv
            o[i] = nv
        return o

    def generate_n_bits_from_store(self, n=1000):
        return self.pre[self.idx:self.idx+n]

    def generate_n_bits(self, n=1000, lookup=True):
        o = np.array([],dtype='int8')
        while n > 0:
            nout = min(n,self.reset_len-self.idx)
            if lookup:
                o = np.concatenate([o, self.generate_n_bits_from_store(nout)])
            else:
                o = np.concatenate([o, self.generate_n_bits_and_store(nout)])
            self.idx += nout
            n -= nout
            if self.idx == self.reset_len:
                self.reg[:] = self.init_reg[:]
                self.idx = 0
        return o

    def generate_n_bits_after_x(self, n=1000, x=0):
        """Generates the next n bits after x bits have been produced"""
        o = np.array([], dtype='uint8')
        idx = x % self.reset_len
        while n > 0:
            nout = min(n, self.reset_len - idx)
            o = np.concatenate([o, self.pre[idx:idx+n]])
            idx += nout
            n -= nout
            if idx == self.reset_len:
                idx = 0
        return o


if __name__ == "__main__":
    m = PRBSGenerator()
    print(m.generate_n_bits(50))
    print(m.generate_n_bits_after_x(10, 40))
