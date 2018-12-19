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

import time
import numpy as np
from gnuradio import gr, gr_unittest
from gnuradio import blocks
import pmt
from prbs_sink_pdu import prbs_sink_pdu
import prbs_generator
from utils import pack_bits


class qa_prbs_sink_pdu (gr_unittest.TestCase):
    PRBS_RESET_LEN = 50
    NUM_GENERATED_PACKETS = 200
    PACKET_LENGTH_BYTES = 8

    def setUp(self):
        self.tb = gr.top_block()

        self.generator = prbs_generator.PRBSGenerator(reset_len=self.PRBS_RESET_LEN)
        self.packets = [self.generator.generate_n_bits(self.PACKET_LENGTH_BYTES*8) for _ in range(self.NUM_GENERATED_PACKETS)]
        self.packets = [pack_bits(unpacked_arr) for unpacked_arr in self.packets]
        self.packets = [
            np.concatenate(
                (
                    np.frombuffer(np.array([i], dtype='<u4').tobytes(), dtype=np.uint8),
                    payload
                )
            ) for (i, payload) in enumerate(self.packets)
        ]

    def tearDown(self):
        self.tb = None

    def test_001_receive_all_correct(self):
        snk = prbs_sink_pdu(self.PACKET_LENGTH_BYTES*8, self.PRBS_RESET_LEN, self.NUM_GENERATED_PACKETS)

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)

        self.tb.msg_connect((src, 'strobe'), (snk, 'all'))
        self.tb.msg_connect((src, 'strobe'), (snk, 'corrected'))

        self.tb.start()
        for packet in self.packets:
            snk.to_basic_block()._post(pmt.intern('all'), pmt.cons(pmt.PMT_NIL, pmt.to_pmt(packet)))
            snk.to_basic_block()._post(pmt.intern('corrected'), pmt.cons(pmt.PMT_NIL, pmt.to_pmt(packet)))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.statistics["Expected number of packets sent"], 200)
        self.assertEqual(snk.statistics["Total number of packets received"], 200)
        self.assertEqual(snk.statistics["Total number of correct packets after FEC"], 200)
        self.assertEqual(snk.statistics["Total number of wrong length packets after FEC (This should be 0)"], 0)
        self.assertEqual(snk.statistics["Total number of erroneous packets after FEC (This should be 0)"], 0)
        self.assertEqual(snk.statistics["Total number of unique packets after FEC"], 200)
        self.assertEqual(snk.statistics["Total number of duplicates"], 0)
        self.assertAlmostEqual(snk.statistics["Frame error rate"], 0)

    def test_001_miss_some_packets(self):
        snk = prbs_sink_pdu(self.PACKET_LENGTH_BYTES*8, self.PRBS_RESET_LEN, self.NUM_GENERATED_PACKETS)

        # We just need something connected to the trimmer block for
        # the flowgraph to compile, but we'll post messages to it directly
        src = blocks.message_strobe(pmt.PMT_NIL, 9999999)

        self.tb.msg_connect((src, 'strobe'), (snk, 'all'))
        self.tb.msg_connect((src, 'strobe'), (snk, 'corrected'))

        self.tb.start()
        for packet in self.packets[:180]:
            snk.to_basic_block()._post(pmt.intern('all'), pmt.cons(pmt.PMT_NIL, pmt.to_pmt(packet)))
            snk.to_basic_block()._post(pmt.intern('corrected'), pmt.cons(pmt.PMT_NIL, pmt.to_pmt(packet)))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(snk.statistics["Expected number of packets sent"], 200)
        self.assertEqual(snk.statistics["Total number of packets received"], 180)
        self.assertEqual(snk.statistics["Total number of correct packets after FEC"], 180)
        self.assertEqual(snk.statistics["Total number of wrong length packets after FEC (This should be 0)"], 0)
        self.assertEqual(snk.statistics["Total number of erroneous packets after FEC (This should be 0)"], 0)
        self.assertEqual(snk.statistics["Total number of unique packets after FEC"], 180)
        self.assertEqual(snk.statistics["Total number of duplicates"], 0)
        self.assertAlmostEqual(snk.statistics["Frame error rate"], 0.1)


if __name__ == '__main__':
    gr_unittest.run(qa_prbs_sink_pdu, "qa_prbs_sink_pdu.xml")
