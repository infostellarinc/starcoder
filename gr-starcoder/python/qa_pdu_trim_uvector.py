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
from pdu_trim_uvector import pdu_trim_uvector
import time
import numpy as np
import pmt


class qa_pdu_trim_uvector (gr_unittest.TestCase):
    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_no_change(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(orig_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(0, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_002_trim_from_start(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([3.3, 4.4, 5.5])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(2, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_003_trim_from_end(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([1.1, 2.2, 3.3])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(0, 2)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_004_trim_from_both_sides(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([2.2, 3.3])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(1, 2)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_005_overflow_from_beginning(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(10, 0)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_006_overflow_from_end(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(0, 10)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_007_overflow_from_both(self):
        orig_array = np.array([1.1, 2.2, 3.3, 4.4, 5.5])
        expected_out_array = np.array([])
        metadata = {'a': 1, 'b': 2}
        in_pmt = pmt.cons(pmt.to_pmt(metadata),
                          pmt.to_pmt(orig_array))
        expected_pmt = pmt.cons(pmt.to_pmt(metadata),
                                pmt.to_pmt(expected_out_array))

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(4, 4)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), in_pmt)
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0), expected_pmt))

    def test_008_ignore_non_pdus(self):
        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)
        trimmer = pdu_trim_uvector(1, 2)
        snk = blocks.message_debug()

        self.tb.msg_connect((src, 'strobe'), (trimmer, 'in'))
        self.tb.msg_connect((trimmer, 'out'), (snk, 'store'))

        self.tb.start()
        trimmer.to_basic_block()._post(pmt.intern('in'), pmt.PMT_NIL)
        trimmer.to_basic_block()._post(pmt.intern('in'), pmt.cons(pmt.PMT_T,
                                                                  pmt.make_u8vector(1, 2)))
        trimmer.to_basic_block()._post(pmt.intern('in'), pmt.cons(pmt.make_dict(),
                                                                  pmt.PMT_T))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 0)


if __name__ == '__main__':
    gr_unittest.run(qa_pdu_trim_uvector, "qa_pdu_trim_uvector.xml")
