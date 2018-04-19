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


expected_decimation = (153, 165, 168, 165, 154, 166, 169, 167, 156, 169, 173,
                       172, 162, 176, 182, 184, 184, 184, 182, 176, 162, 172,
                       173, 169, 157, 167, 169, 166, 154, 165, 168, 165)

expected_max_hold = (153, 168, 168, 168, 169, 169, 169, 170, 157, 172, 173,
                     174, 176, 179, 182, 184, 184, 184, 182, 179, 176, 174,
                     173, 172, 157, 170, 169, 169, 169, 168, 168, 168)


class qa_waterfall_heatmap (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.fft_size = 32

    def tearDown (self):
        self.tb = None

    def generate_data(self):
        return tuple([complex(i, i+100) * 0.00001 for i in range(100)] * 20)

    def test_001_decimation_mode(self):
        src_data = self.generate_data()
        src = blocks.vector_source_c(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, self.fft_size)
        op = starcoder.waterfall_heatmap(1000, 0, 1, self.fft_size, 0)
        dst = blocks.vector_sink_b(self.fft_size)
        self.tb.connect(src, s2v, op, dst)

        self.tb.run ()
        self.assertEqual(dst.data(), expected_decimation)

    def test_001_max_hold_mode(self):
        src_data = self.generate_data()
        src = blocks.vector_source_c(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, self.fft_size)
        op = starcoder.waterfall_heatmap(1000, 0, 1, self.fft_size, 1)
        dst = blocks.vector_sink_b(self.fft_size)
        self.tb.connect(src, s2v, op, dst)

        self.tb.run ()
        self.assertEqual(dst.data(), expected_max_hold)


if __name__ == '__main__':
    gr_unittest.run(qa_waterfall_heatmap, "qa_waterfall_heatmap.xml")
