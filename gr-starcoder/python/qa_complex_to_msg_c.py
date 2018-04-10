#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 InfoStellar, Inc.
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
import pmt
import numpy as np
import starcoder_swig as starcoder

class qa_complex_to_msg_c (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_single_message_out (self):
        src_data = (1j, 1+2j, 1)
        # expected is the binary representation of (1j, 1+2j)
        expected = np.array([0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 128, 63, 0, 0, 0, 64], dtype='uint8')

        src = blocks.vector_source_c(src_data)
        ctm = starcoder.complex_to_msg_c(2)
        snk = blocks.message_debug()
        self.tb.connect(src, ctm)
        self.tb.msg_connect((ctm, 'out'), (snk, 'store'))
        self.tb.run()

        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.is_blob(snk.get_message(0)))
        np.testing.assert_array_equal(pmt.to_python(snk.get_message(0)), expected)

    def test_002_multiple_message_out (self):
        src_data = (1j, 1+2j, 1, 0)
        # expected1 is the binary representation of (1j, 1+2j)
        expected1 = np.array([0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 128, 63, 0, 0, 0, 64], dtype='uint8')
        # expected2 is the binary representation of (1, 0)
        expected2 = np.array([0, 0, 128, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], dtype='uint8')

        src = blocks.vector_source_c(src_data)
        ctm = starcoder.complex_to_msg_c(2)
        snk = blocks.message_debug()
        self.tb.connect(src, ctm)
        self.tb.msg_connect((ctm, 'out'), (snk, 'store'))
        self.tb.run()

        self.assertEqual(snk.num_messages(), 2)
        self.assertTrue(pmt.is_blob(snk.get_message(0)))
        self.assertTrue(pmt.is_blob(snk.get_message(1)))
        np.testing.assert_array_equal(pmt.to_python(snk.get_message(0)), expected1)
        np.testing.assert_array_equal(pmt.to_python(snk.get_message(1)), expected2)

if __name__ == '__main__':
    gr_unittest.run(qa_complex_to_msg_c, "qa_complex_to_msg_c.xml")
