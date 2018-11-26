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

class qa_golay_decoder(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block ()

    def tearDown(self):
        self.tb = None

    def normal(self):
        # set up fg
        decoder = starcoder.golay_decoder(0, 1)
        snk = blocks.message_debug()

        self.tb.msg_connect((decoder, 'out'), (snk, 'store'))

        self.tb.start()
        decoder._post(pmt.intern('in'),
                      pmt.cons(pmt.make_dict(), pmt.make_u8vector(144, 0x34)))
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        self.assertEqual(snk.num_messages(), 1)
        self.assertTrue(pmt.equal(snk.get_message(0),
                                  pmt.cons(pmt.PMT_NIL, pmt.make_u8vector(120, 0x34))))

if __name__ == '__main__':
    gr_unittest.run(qa_golay_decoder, "qa_golay_decoder.xml")
