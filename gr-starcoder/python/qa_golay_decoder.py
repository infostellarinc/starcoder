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
import copy
import pmt
import starcoder_swig as starcoder
import time

PREAMBLE = [0x32, 0x6F, 0x19, 0xD3]
POSTAMBLE = [0x77, 0x10, 0xDE, 0xF1]

MESSAGE1 = [1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1]
ENCODED1 = [0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1]

MESSAGE2 = [0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0]
ENCODED2 = [1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0]

class qa_golay_decoder(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block ()
        self.decoder = starcoder.golay_decoder(4, 2)
        self.snk = blocks.message_debug()
        self.tb.msg_connect((self.decoder, 'out'), (self.snk, 'store'))

    def tearDown(self):
        self.tb = None

    def test_noError(self):
        input_vector = pmt.init_u8vector(
            56, PREAMBLE + ENCODED1 + ENCODED2 + POSTAMBLE)
        message_car = pmt.dict_add(pmt.make_dict(), pmt.intern('key'), pmt.intern('value'))

        self.tb.start()
        self.decoder._post(pmt.intern('in'),
                           pmt.cons(message_car, input_vector))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(self.snk.num_messages(), 1)
        expected_vector = pmt.init_u8vector(
            32, PREAMBLE + MESSAGE1 + MESSAGE2 + POSTAMBLE)
        self.assertTrue(pmt.equal(self.snk.get_message(0),
                                  pmt.cons(message_car, expected_vector)))

    def test_errorCorrected(self):
        encoded1 = copy.copy(ENCODED1)
        encoded1[3] = 1 - encoded1[3]
        encoded1[12] = 1 - encoded1[12]
        encoded1[17] = 1 - encoded1[17]
        encoded2 = copy.copy(ENCODED2)
        encoded2[7] = 1 - encoded2[7]
        encoded2[10] = 1 - encoded2[10]
        encoded2[11] = 1 - encoded2[11]
        input_vector = pmt.init_u8vector(
            56, PREAMBLE + encoded1 + encoded2 + POSTAMBLE)
        message_car = pmt.dict_add(pmt.make_dict(), pmt.intern('key'), pmt.intern('value'))

        self.tb.start()
        self.decoder._post(pmt.intern('in'),
                      pmt.cons(message_car, input_vector))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(self.snk.num_messages(), 1)
        expected_vector = pmt.init_u8vector(
            32, PREAMBLE + MESSAGE1 + MESSAGE2 + POSTAMBLE)
        self.assertTrue(pmt.equal(self.snk.get_message(0),
                                  pmt.cons(message_car, expected_vector)))

    def test_ignoreFourErrorBits(self):
        encoded1 = copy.copy(ENCODED1)
        encoded1[3] = 1 - encoded1[3]
        encoded1[12] = 1 - encoded1[12]
        encoded1[17] = 1 - encoded1[17]
        encoded1[19] = 1 - encoded1[19]
        encoded2 = copy.copy(ENCODED2)
        encoded2[7] = 1 - encoded2[7]
        encoded2[10] = 1 - encoded2[10]
        encoded2[11] = 1 - encoded2[11]
        encoded2[22] = 1 - encoded2[22]
        input_vector = pmt.init_u8vector(
            56, PREAMBLE + encoded1 + encoded2 + POSTAMBLE)

        self.tb.start()
        self.decoder._post(pmt.intern('in'),
                      pmt.cons(pmt.make_dict(), input_vector))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(self.snk.num_messages(), 0)

    def test_correctFiveErrorBitsToDifferentMessage(self):
        encoded1 = copy.copy(ENCODED1)
        encoded1[3] = 1 - encoded1[3]
        encoded1[5] = 1 - encoded1[5]
        encoded1[12] = 1 - encoded1[12]
        encoded1[17] = 1 - encoded1[17]
        encoded1[19] = 1 - encoded1[19]
        encoded2 = copy.copy(ENCODED2)
        encoded2[7] = 1 - encoded2[7]
        encoded2[10] = 1 - encoded2[10]
        encoded2[11] = 1 - encoded2[11]
        encoded2[18] = 1 - encoded2[18]
        encoded2[22] = 1 - encoded2[22]
        input_vector = pmt.init_u8vector(
            56, PREAMBLE + encoded1 + encoded2 + POSTAMBLE)
        message_car = pmt.dict_add(pmt.make_dict(), pmt.intern('key'), pmt.intern('value'))

        self.tb.start()
        self.decoder._post(pmt.intern('in'),
                      pmt.cons(message_car, input_vector))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        self.assertEqual(self.snk.num_messages(), 1)
        expected_vector = pmt.init_u8vector(
            32, PREAMBLE + MESSAGE1 + MESSAGE2 + POSTAMBLE)
        self.assertFalse(pmt.equal(self.snk.get_message(0),
                                   pmt.cons(message_car, expected_vector)))

if __name__ == '__main__':
    gr_unittest.run(qa_golay_decoder, "qa_golay_decoder.xml")
