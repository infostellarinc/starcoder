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
from add_incrementing_header_pdu import add_incrementing_header_pdu
import numpy as np
import pmt
import time


class qa_add_incrementing_header_pdu (gr_unittest.TestCase):
    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_no_change(self):
        original_arrays = [
            np.array([1, 2, 3, 4, 5], dtype=np.uint8),
            np.array([6, 7, 8, 9, 10], dtype=np.uint8),
            np.array([11, 12, 13, 14, 15], dtype=np.uint8)
        ]

        expected_arrays = [
            np.array([0, 0, 0, 0, 1, 2, 3, 4, 5], dtype=np.uint8),
            np.array([0, 0, 0, 1, 6, 7, 8, 9, 10], dtype=np.uint8),
            np.array([0, 0, 0, 2, 11, 12, 13, 14, 15], dtype=np.uint8)
        ]

        metadata = {'a': 1, 'b': 2}

        in_pmts = [pmt.cons(pmt.to_pmt(metadata), pmt.to_pmt(arr)) for arr in original_arrays]
        expected_pmts = [pmt.cons(pmt.to_pmt(metadata), pmt.to_pmt(arr)) for arr in expected_arrays]

        appender = add_incrementing_header_pdu()
        snk = blocks.message_debug()

        self.tb.msg_connect((appender, 'out'), (snk, 'store'))

        self.tb.start()
        [appender.to_basic_block()._post(pmt.intern('in'), in_pmt) for in_pmt in in_pmts]
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 3)
        [self.assertTrue(pmt.equal(snk.get_message(i), expected) for (i, expected) in enumerate(expected_pmts))]

    def test_002_ignore_non_u8_pdu(self):
        appender = add_incrementing_header_pdu()
        snk = blocks.message_debug()

        self.tb.msg_connect((appender, 'out'), (snk, 'store'))

        self.tb.start()
        appender.to_basic_block()._post(pmt.intern('in'), pmt.PMT_NIL)
        appender.to_basic_block()._post(pmt.intern('in'), pmt.cons(pmt.PMT_T,
                                                                   pmt.make_u8vector(1, 2)))
        appender.to_basic_block()._post(pmt.intern('in'), pmt.cons(pmt.make_dict(),
                                                                   pmt.PMT_T))
        appender.to_basic_block()._post(pmt.intern('in'), pmt.cons(
            pmt.make_dict(),
            pmt.to_pmt(np.array([1, 0, 1, 0, 1, 0, 1], dtype=np.int32))))

        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 0)


if __name__ == '__main__':
    gr_unittest.run(qa_add_incrementing_header_pdu, "qa_add_incrementing_header_pdu.xml")
