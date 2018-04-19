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

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import starcoder_swig as starcoder
import os
import tempfile


class qa_waterfall_plotter (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.fft_size = 40
        _, path = tempfile.mkstemp('.png')
        self.filename = path

    def tearDown (self):
        self.tb = None
        try:
            os.remove(self.filename)
        except OSError:
            pass

    def test_001_t (self):
        src_data = tuple(range(140, 180)*20)
        src = blocks.vector_source_b(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_char, self.fft_size)
        op = starcoder.waterfall_plotter(1, 0, 1, self.fft_size, self.filename)
        self.tb.connect(src, s2v, op)
        self.tb.run()
        with open(self.filename) as f:
            with open("test_waterfall.png") as g:
                self.assertEqual(f.read(), g.read())


if __name__ == '__main__':
    gr_unittest.run(qa_waterfall_plotter, "qa_waterfall_plotter.xml")
