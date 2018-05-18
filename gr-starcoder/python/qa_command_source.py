#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 <+YOU OR YOUR COMPANY+>.
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
import time
import starcoder_pb2

class qa_command_source (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        cs = starcoder.command_source()
        snk = blocks.message_debug()
        self.tb.msg_connect((cs, 'out'), (snk, 'print'))
        self.tb.start()
        msg = starcoder_pb2.PMT()
        msg.pmt_pair_value.cdr.symbol_value = "testtransmission"
        msg.pmt_pair_value.car.double_value = 23.2
        cs.push(msg.SerializeToString())
        time.sleep(1)
        self.tb.stop()
        self.tb.wait()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_command_source, "qa_command_source.xml")
