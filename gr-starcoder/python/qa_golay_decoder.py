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
import starcoder_swig as starcoder
import time

PREAMBLE = [0x32, 0x6F, 0x19, 0xD3]
POSTAMBLE = [0x77, 0x10, 0xDE, 0xF1]

ENCODED1 = [1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1]
DECODED1 = [1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1]

ENCODED2 = [1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0]
DECODED2 = [0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0]

class qa_golay_decoder(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block ()
        self.decoder = starcoder.golay_decoder(4, 2)
        self.snk = blocks.message_debug()
        self.tb.msg_connect((self.decoder, 'out'), (self.snk, 'store'))

    def tearDown(self):
        self.tb = None

    def test_noError(self):
        self.tb.start()
        input_vector = pmt.init_u8vector(
            56, PREAMBLE + ENCODED1 + ENCODED2 + POSTAMBLE)
        self.decoder._post(pmt.intern('in'),
                      pmt.cons(pmt.make_dict(), input_vector))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(self.snk.num_messages(), 1)
        expected_vector = pmt.init_u8vector(
            32, PREAMBLE + DECODED1 + DECODED2 + POSTAMBLE)
        self.assertTrue(pmt.equal(self.snk.get_message(0),
                                  pmt.cons(pmt.PMT_NIL, expected_vector)))

if __name__ == '__main__':
    gr_unittest.run(qa_golay_decoder, "qa_golay_decoder.xml")
