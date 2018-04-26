#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 Infostellar, Inc.
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
import pmt
import time

class qa_enqueue_message_sink (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # The following source block is needed even if we don't use its message.
        # This is because the GNURadio scheduler will not create a thread for
        # blocks that aren't connected to anything. Thus the following block is
        # needed to have enqueue_message_sink connected to something.
        src = blocks.message_strobe(pmt.intern('unused message'), 99999)
        op = starcoder.enqueue_message_sink()

        msg1 = pmt.make_u8vector(3, ord('A'))
        msg2 = pmt.make_u8vector(20, ord('b'))

        op.to_basic_block()._post(pmt.intern('in'), msg1)
        op.to_basic_block()._post(pmt.intern('in'), msg2)

        self.tb.msg_connect(src, 'strobe', op, 'in')
        self.tb.start()
        # This sleep is needed since GNURadio messages are handled
        # asynchronously by the GNURadio scheduler. Unfortunately, even the
        # tests for message sinks in the gnuradio repository itself uses sleeps.
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # Check that the passed PMT blobs are accessible from starcoder_observe
        # in the expected serialized binary format.
        # self.assertEqual(op.starcoder_observe(), pmt.serialize_str(msg1))
        # self.assertEqual(op.starcoder_observe(), pmt.serialize_str(msg2))

        # Check that after retrieving all available messages, starcoder_observe
        # returns the empty string.
        # self.assertEqual(op.starcoder_observe(), '')
        # self.assertEqual(op.starcoder_observe(), '')


if __name__ == '__main__':
    gr_unittest.run(qa_enqueue_message_sink, "qa_enqueue_message_sink.xml")
